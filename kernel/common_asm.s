;/kernel/common_asm.s
;by 不吃香菜的大头怪
;2020-10-30 create
;2020-10-30 last modify
;Description:  Provide Some Common Tool Function Which Must Write By ASM
[BITS 32]
section .text

;------------------EXTERN------------------
[EXTERN gdt_ptr]
;-----------------------------------------------


;------------------GLOBAL-------------------
[GLOBAL reload_gdt]
;------------------------------------------------

reload_gdt:
    lgdt [gdt_ptr]
    ret