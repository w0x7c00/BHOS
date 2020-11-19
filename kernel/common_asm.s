;/kernel/common_asm.s
;by 不吃香菜的大头怪
;2020-10-30 create
;2020-10-30 last modify
;Description:  Provide Some Common Tool Function Which Must Write By ASM
[BITS 32]
section .text

;------------------EXTERN------------------
[EXTERN gdt_ptr]
[EXTERN print_debug_1]
;-----------------------------------------------


;------------------GLOBAL-------------------
[GLOBAL reload_gdt]
[GLOBAL exit_int]
[GLOBAL reload_kern_page_table]
[GLOBAL get_eflag]
;------------------------------------------------

;void get_eflag(uint32_t *eflag);
get_eflag:
    ;get first parame
    mov eax,[esp+4]
    push ebx
    pushf
    pop ebx
    mov [eax],ebx
    pop ebx
    ret

reload_gdt:
    lgdt [gdt_ptr]
    ret

;parame1   pdt_paddr
;origin format:    void reload_kern_page_table(uint32_t pdt_paddr)
reload_kern_page_table:
    ;get parame 1 from [esp+4]
    mov eax,[esp+4]
    and eax,0xFFFFF000
    ;relaod cr3
    mov cr3,eax
    ret

;参数1   esp的值
;C语言原型     void *  exit_int(void* esp)     esp必须是栈指针（地址）
exit_int:
    mov eax,[esp+4]
    mov esp,eax       ;修改栈位置
    ;以下部分是模拟中断中的执行返回(见interrupt_asm.s)

    add esp,8
	pop eax
	mov gs,ax
	pop eax
	mov fs,ax
	pop eax
	mov es,ax
	popad
	add esp,8



;由于start_user_task函数执行时一定是因为时钟中断的线程调度  调度完成以后就已经打开了中断并且设置了8259A   
;执行到此步时无需再次打开8259A   
	;mov al,0x20
	;out 0xA0,al
	;out 0x20,al
	iret

