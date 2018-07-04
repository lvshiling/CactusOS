
# sudo apt-get install g++ binutils libc6-dev-i386
# sudo apt-get install VirtualBox grub-legacy xorriso

GCCPARAMS = -m32 -g -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-exceptions -fno-rtti -fno-leading-underscore -Wno-write-strings -fpermissive
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = obj/loader.o \
		  obj/common/convert.o \
		  obj/common/math.o \
		  obj/common/memoryoperations.o \
		  obj/core/gdtloader.o \
		  obj/core/gdt.o \
		  obj/core/interruptstubs.o \
		  obj/core/interrupts.o \
		  obj/core/rtc.o \
		  obj/core/pit.o \
		  obj/core/memorymanagement.o \
		  obj/core/sse.o \
		  obj/core/cpu.o \
		  obj/core/smbios.o \
		  obj/core/pci.o \
		  obj/system.o \
          obj/kernel.o


run: CactusOS.iso
	(killall VirtualBox && sleep 1) || true
	virtualbox --startvm 'CactusOS' 

obj/%.o: src/%.cpp
	mkdir -p $(@D)
	i686-elf-g++ $(GCCPARAMS) -c -o $@ $<

obj/%.o: src/%.s
	mkdir -p $(@D)
	i686-elf-as $(ASPARAMS) -o $@ $<

#NASM assembly files

obj/core/sse.o: src/core/sse.s
	nasm -f elf $< -o $@

#

CactusOS.bin: linker.ld $(objects)
	i686-elf-ld $(LDPARAMS) -T $< -o $@ $(objects)

CactusOS.iso: CactusOS.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp CactusOS.bin iso/boot/CactusOS.bin
	cp grub.cfg iso/boot/grub/grub.cfg #Use a existing grub config file
	grub-mkrescue --output=CactusOS.iso iso
	rm -rf iso

install: CactusOS.iso
	cp $< /media/sf_Mint_OSDev/CactusOS.iso
	cp CactusOS.bin /media/sf_Mint_OSDev/CactusOS.bin

.PHONY: clean qemu
clean:
	rm -rf obj CactusOS.bin CactusOS.iso

qemu: CactusOS.iso
	qemu-system-x86_64 -cdrom CactusOS.iso