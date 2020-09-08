#Makefile for BHOS
#edit:2020/1/26
#by 不吃香菜的大头怪

C_SOURCES = $(shell find . -name "*.c")        #.c源文件
C_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES)) #.c生成.o文件
S_SOURCES = $(shell find . -name "*.s")        #.s源文件
S_OBJECTS = $(patsubst %.s, %.o, $(S_SOURCES)) #.s生成.o文件


#编译器与链接器参数
nasm_pars = -f elf -g -F stabs
gcc_pars = -c -Wall -m32 -ggdb -gstabs+ -nostdinc -fno-builtin -fno-stack-protector -I include
ld_pars = -T kernel.ld -m elf_i386 -nostdlib

#目标软盘
target_floppy = floppy.img

#目标硬盘
target_hd = hd.img
#目标回环设备
target_loop = 23

#过程
all: $(S_OBJECTS) $(C_OBJECTS) link mt copykern umt

.c.o:
	gcc $(gcc_pars) $< -o $@

.s.o:
	nasm $(nasm_pars) $<

.PHONY:link
link:
	ld $(ld_pars) $(S_OBJECTS) $(C_OBJECTS) -o kernel.elf

.PHONY:copykern
copykern:
	sudo cp kernel.elf /mnt/kernel
	
.PHONY:mt
mt:
	#sudo mount $(target_floppy) /mnt/kernel
	sudo losetup /dev/loop$(target_loop) $(target_hd) -o 32256
	sudo mount /dev/loop$(target_loop) /mnt/kernel -t ext3 -o loop
	

.PHONY:umt
umt:
	sudo umount /mnt/kernel
	sudo losetup -d /dev/loop$(target_loop)


.PHONY:run
run:
	#qemu -fda $(target_floppy) -boot a
	qemu -hda $(target_hd) -boot a

.PHONY:clean
clean:
	rm $(S_OBJECTS) $(C_OBJECTS) kernel.elf

.PHONY:debug
debug:
	qemu -S -s -hda $(target_hd) -boot a &
	sleep 1;gdb -x gdb.script


.PHONY:tui_debug
tui_debug:
	qemu -S -s -hda $(target_hd) -boot a &
	sleep 1;gdb -tui -x gdb.script

.PHONY:simple_debug
simple_debug:
	qemu -S -s -hda $(target_hd) -boot a &
	sleep 1

.PHONY:gdbgui
gdbgui:
	qemu -S -s -hda $(target_hd) -boot a & gdbgui -r 127.0.0.1:1234  --gdb-args="-x gdb.script"

.PHONY:bochs
bochs:
	bochs -f bochsrc
