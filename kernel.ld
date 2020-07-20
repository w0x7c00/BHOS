/*
全局链接脚本
by 不吃香菜的大头怪
last edit：2020/7/18
*/
/*注意变量与赋值符号之间要有空格*/
ENTRY(boot_start)
SECTIONS
{
    /*内核是加载到1 M空间之上的*/
    PROVIDE( kern_start =0x00100000);
    . = 0x100000;
        .init.text : 
    {
        *(.init.text)
        . = ALIGN(4096);
    }
    .init.data : 
    {
        *(.init.data)
        . = ALIGN(4096);
    }

    . += 0xC0000000;
    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)
        . = ALIGN(4096);
    }

    .data : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
        . = ALIGN(4096);
    }
    .rodata : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata)
        . = ALIGN(4096);    
    }
    .bss : AT(ADDR(.bss) - 0xC0000000)
    {
        *(.bss)
        . = ALIGN(4096);
    }

    .stab : AT(ADDR(.stab) - 0xC0000000)
    {
        *(.stab)
        . = ALIGN(4096);
    }
    
    .stabstr : AT(ADDR(.stabstr) - 0xC0000000)
    {
        *(.stabstr)
        . = ALIGN(4096);
    }
    . -= 0xC0000000;
    PROVIDE( kern_end =.);
    /DISCARD/ : { *(.comment) *(.eh_frame) }
}