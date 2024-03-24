#include "./string.h"

void *memchr(const void *str, int c, size_t n){
    while(*(char *)(str)){
        if(*(unsigned char *)str == (unsigned char)c){
            return (void *)str;
        }
        str++;
    }
    return NULL;
}
int memcmp(const void *str1, const void *str2, size_t n){
    for(int i = 0; i < n; i++){
        if(*(char *)(str1 + i) != *(char *)(str2 + i)) return *(char *)(str1 + i) - *(char *)(str2 + i);
    }
    return 0;
}
void *memcpy(void *restrict dest, const void *restrict src, size_t n){
    for(int i = 0; i < n; i++){
        *(unsigned char*)(dest + i) = *(unsigned char*)(src + i);
    }
    return dest;
}
void *memmove(void *dest, const void *src, size_t n){
    for(int i = 0; i < n; i++){
        *(unsigned char*)(dest + i) = *(unsigned char*)(src + i);
    }
    return dest;
}
void *memset(void *dest, int c, size_t n){
    for(int i = 0; i < n/4; i++){
        ((unsigned int*)dest)[i] = c;
    }
    return dest;
}
char *strcat(char *restrict dest, const char *restrict src){
    void *base = dest;
    while(*dest != 0)dest++;
    while(*src){
        *(dest++) = *(src++);
    }
    *dest = 0;
    return base;
}
size_t strlen(const char *str){
    size_t size = 0;
    while(*str++) size++;
    return size;
}
int strcmp(const char *str1, const char *str2){
    while(*str1){
        if(*str1 != *str2) return *str1 - *str2;
        str1++;
        str2++;
    }
    return 0;
}