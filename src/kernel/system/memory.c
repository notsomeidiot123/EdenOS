#include "memory.h"
#include <stdint.h>
#include "log.h"

uint64_t max_memory = 0;

uint8_t *phys_bitmap = 0;
uint32_t pmbm_start = 0;

uint32_t pm_alloc(){
    for(uint32_t i = pmbm_start; i < max_memory/4096; i++){
        if(!phys_bitmap[i]){
            phys_bitmap[i] = 1;
            return i * 4096;
        }
    }
    return 0;
}
void pm_free(uint32_t paddr){
    phys_bitmap[paddr/4096] = 0;
}

void set_cr3(uint32_t pd_addr){
    asm volatile("mov %0, %%cr3" : : "r" (pd_addr));
}

uint64_t get_cr2(){
    uint32_t cr2;
    asm volatile("mov %%cr3, %0" : "=r"(cr2));
    return cr2;
}

uint64_t get_cr3(){
    uint32_t cr3_paddr;
    asm volatile("mov %%cr3, %0" :"=r"(cr3_paddr));
    return cr3_paddr;
}
void invlpg(void *addr){
    asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

int32_t map(uint32_t paddr, void *vaddr, uint16_t attributes){
    uint64_t pd_index = (uint64_t)vaddr >> 22 & 0x3ff;
    uint64_t pt_index = (uint64_t)vaddr >> 12 & 0x3ff;
    
    pde_t *pd = (void *)0xfffff000;
    pte_t *pt = (void *)((uint64_t)0xffc00000 + (0x1000 * pd_index));
    if(!pd[pd_index].addr){
        pd[pd_index].addr = pm_alloc()/4096;
        pd[pd_index].present = 1;
        pd[pd_index].read_write = 1;
        pd[pd_index].user = attributes & PG_ATTR_USER;
        for(uint32_t i = 0; i < 1024; i++){
            pt[i].addr = 0;
            pt[i].used = 0;
            pt[i].present = 0;
        }
        set_cr3(get_cr3());
    }
    pd[pd_index].user |= attributes & PG_ATTR_USER;
    invlpg(pt);
    invlpg(pd);
    if(pt[pt_index].used) return -1;
    *(uint32_t *)(&pt[pt_index]) = paddr | attributes | PG_ATTR_USED | PG_ATTR_USED;
    pt[pt_index].addr = paddr >> 12;
    pt[pt_index].used = 1;
    pt[pt_index].present = 1;
    //debug printing
    // kprintf("[[%0x]]", vaddr);
    // kprintf("[[%0x]]\n", *(uint32_t *)(&pt[pt_index]));
    set_cr3(get_cr3());
    return 0;
}
void unmap(void *vaddr){
    uint64_t pd_index = (uint64_t)vaddr >> 22;
    uint64_t pt_index = (uint64_t)vaddr >> 12 & 0x3ff;
    
    pde_t *pd = (void *)0xfffff000;
    pte_t *pt = (void *)((uint64_t)0xffc00000 + (0x400 * pd_index));
    if(!pd[pd_index].present) return;
    pm_free(pt[pt_index].addr << 12);
    *(uint32_t *)(&pt[pt_index]) = 0;
    invlpg(vaddr);
}

pte_t get_paddr(void *vaddr){
    uint64_t pd_index = (uint64_t)vaddr >> 22;
    uint64_t pt_index = (uint64_t)vaddr >> 12 & 0x3ff;
    
    pde_t *pd = (void *)0xfffff000;
    pte_t *pt = (void *)(0xffc00000 + (0x400 * pd_index));
    
    if(!pd[pd_index].present) return (pte_t){0};
    return pt[pt_index];
}

void vfree(void *vaddr){
    pte_t entry = get_paddr(vaddr);
    if(!entry.used){
        return;
    }
    while(entry.link_last){
        vaddr -= 4096;
        entry = get_paddr(vaddr);
    }
    do{
        unmap(vaddr);
        vaddr +=4096;
        entry = get_paddr(vaddr);
    }while(entry.link_next);
}

void *valloc(uint64_t size, uint32_t attributes){
    //because the first MB is ID mapped, we can start after that
    uint64_t count = (size/4096) + 1;
    for(uint64_t i = 0; i < 0xffffffff/4096; i++){
        if(get_paddr((void *)(i * 4096)).used){
            continue;
        }
        uint32_t j = 0;
        while(j < count){
            if(get_paddr((void *)((i + j) * 4096)).used) break;
            j++;
        }
        if( j >= count ){
            for(int k = 0; k <= count; k++){
                uint32_t paddr = pm_alloc();
                if(paddr == 0){
                    for(int l = 0; l < k; l++){
                        unmap((void *)((i + l) * 4096));
                    }
                    return (void *)0;//out of memory
                }
                map(paddr, (void *)((i + k) * 4096), attributes | PG_ATTR_USED | ((k > 0) * PG_ATTR_LINK_LAST) | (k < count) * PG_ATTR_LINK_NEXT);
            }
            return (void *)(i * 4096);
        }
    }
    return 0;
}

void *valloc_pages(uint64_t size, uint32_t attributes){
    //because the first MB is ID mapped, we can start after that
    uint64_t count = size;
    for(uint64_t i = 0; i < 0xffffffff/4096; i++){
        if(get_paddr((void *)(i * 4096)).used){
            continue;
        }
        uint32_t j = 0;
        while(j < count){
            if(get_paddr((void *)((i + j) * 4096)).used) break;
            j++;
        }
        if( j >= count ){
            for(int k = 0; k <= count; k++){
                uint32_t paddr = pm_alloc();
                if(paddr == 0){
                    for(int l = 0; l < k; l++){
                        unmap((void *)((i + l) * 4096));
                    }
                    return (void *)0;//out of memory
                }
                map(paddr, (void *)((i + k) * 4096), attributes | PG_ATTR_USED | ((k > 0) * PG_ATTR_LINK_LAST) | (k < count) * PG_ATTR_LINK_NEXT);
            }
            return (void *)(i * 4096);
        }
    }
    return 0;
}

void memory_init(kernel_info_t *kinfo){
    pde_t *page_dir = (void *)get_cr3();
    *(uint32_t *)(&page_dir[1023]) = get_cr3() | 0x803;
    set_cr3(get_cr3()); //nuke pd just to make sure
    phys_bitmap = kinfo->pmbm;
    mmap_t *mmap = kinfo->mmap;
    kprintf("|TYPE|  BASE  | LENGTH | LIMIT  |\n|----|--------|--------|--------|\n");
    for(int i = 0; i < (1 * 1024 * 1024) / 4096; i++){
        phys_bitmap[i] = 1;
    }
    for(uint32_t i = 0; i < kinfo->mmap_count; i++){
        if(mmap[i].type != MMAP_TYPE_USABLE){
            for(uint32_t j = 0; j < mmap[i].length / 4096; j++){
                phys_bitmap[(mmap[i].base/4096) + j] = 1;
                // map((mmap[i].base) + j, (void *)(mmap[i].base) + (j * 4096), PG_ATTR_GLOBAL);
            }
        }
        else if(mmap[i].type == MMAP_TYPE_USABLE){
            for(uint32_t j = 0; j < mmap[i].length / 4096; j++){
                phys_bitmap[mmap[i].base/4096 + j] = 0;
            }
        }
        uint32_t lim = mmap[i].base + mmap[i].length;
        max_memory = (lim > max_memory) ? lim : max_memory;
        kprintf("| %d  |%0x|%0x|%0x|\n", mmap[i].type, mmap[i].base, mmap[i].base + mmap[i].length, mmap[i].length);
    }
    for(uint32_t i = (uint64_t)phys_bitmap/4096; i < max_memory/4096/4096; i++){
        phys_bitmap[i] = 1;
    }
    for(uint32_t i = 0; i < 0x100000/4096; i++){
        phys_bitmap[i] = 1;
    }
    for(int i = 0; i < kinfo->mmap_count; i++){
        if(i > 0 && mmap[i-1].base + mmap[i-1].length != mmap[i].base && (mmap[i].base + mmap[i].length) < max_memory){
            for(int j = 0; j < (mmap[i].base - mmap[i-1].base + mmap[i-1].length)/4096; j++){
                phys_bitmap[(mmap[i-1].base + mmap[i - 1].length)/4096 + j] = 1;
            }
        }
    }
    while(phys_bitmap[pmbm_start]){
        pmbm_start++;
    }
    // map(pm_alloc(), (void *)0x00400000, 0);
    // uint32_t *test = (void *)0x400000;
    
}