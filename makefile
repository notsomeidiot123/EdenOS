all:
	nasm -f bin src/bootloader/mbr.s -o obj/mbr.bin
	nasm -f bin src/bootloader/bootloader.s -o obj/boot.bin
	nasm -f elf src/kernel/kentry.s -o obj/kentry.o
	nasm -f elf src/kernel/system/idt.s -o src/kernel/obj/idt_asm.o
	
	sh c_build_helper.sh
	
	cat obj/mbr.bin > img.hd
	qemu-img resize img.hd 1M
	cat obj/boot.bin >> img.hd
	cat obj/kernel.bin >> img.hd
	qemu-img resize img.hd 512M
	qemu-system-i386 -hda img.hd -fda fd0.img -no-reboot -no-shutdown -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0 -m 512M -smp 2 -serial mon:stdio -d int -D log.txt -vga std
	
	@echo