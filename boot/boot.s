;/boot/boot.s
;edit:2020/1/26
;by:不吃香菜的大头怪
;多个页目录项不能共享一个页表 这样在修改页表同时 多个页目录的映射也会改变
%include "./boot/boot.inc" 
;这三个双字为GRUB加载器识别MagicNumber 以及配置信息（可以不用理解）
section .init.text       ;这个节在文件开头所以要标示一下
dd 0x1badb002

dd 0x3   	
dd -(0x1badb002+0x3)

temp_mboot_ptr:      ;暂存ebx
    dd 0x0
[BITS 32]        ;GRUB已经帮助我们进入了保护模式 所以是可以使用32编译的

[GLOBAL boot_start]
boot_start:        ;此处是内核加载后调用的第一个函数
    cli         ;关外中断
    mov [temp_mboot_ptr],ebx
    mov esp,INIT_STACK_TOP
    and esp, 0xFFFFFFF0  ;16字节对齐
    call set_page
    mov ebx,[temp_mboot_ptr]
    mov eax,kern_dir_table
    and eax,0xFFFFF000
    mov cr3,eax
    mov eax,cr0

    ;加载页表 启用分页
    or eax,0x80000000
    mov cr0,eax
    jmp dword SELECTOR_CODE:boot_start_after_set_paging

[GLOBAL reload_kern_page]
reload_kern_page:
    mov eax,kern_dir_table
    and eax,0xFFFFF000
    mov cr3,eax
    mov eax,cr0
    ret
set_page:
    mov eax,4095      ;4096-1
    .clear_dir_table:             ;重置页目录表内存空间
    mov  byte[kern_dir_table+eax],0
    dec eax
    jnz .clear_dir_table
    mov byte[kern_dir_table],0

    mov eax,4096*256      ;一张表占用4KB    共256张表
    .clear_page_table:        ;重置256张页表内存空间
    mov byte[kern_page_table+eax],0
    dec eax
    jnz .clear_page_table
    mov byte[kern_page_table],0

    mov ecx,0
    mov cx,255  ;256-1
    .create_pde:            ;创建临时页目录项与页表对应关系    将除了最后一张页表以外的255张页表全部映射到页目录中    页目录所有的项都是present的
    ;mov eax,kern_page_table+ecx*4096     ;1024*4
    mov eax,0
    mov ax,4096
    mul  cx      ;结果在eax
    add eax,kern_page_table
    or eax,PG_US_S|PG_RW_W|PG_P     ;eax存放了一张页表信息    也就是一条页目录项
    mov [kern_dir_table+ecx*4+0xc00],eax
    dec cx
    jnz .create_pde
    mov eax,kern_page_table
    or eax,PG_US_S|PG_RW_W|PG_P   
    mov [kern_dir_table+0xc00],eax
    mov [kern_dir_table],eax    ;映射0x0起始4MB    这一段映射只会使用一次

    mov eax,1023
    .create_pte:              
    mov ebx,eax
    sal ebx,12
    and ebx,0xFFFFF000
    or ebx,PG_US_S|PG_RW_W|PG_P
    mov [kern_page_table+eax*4],ebx
    dec eax
    jnz .create_pte
    mov ebx,0
    or ebx,PG_US_S|PG_RW_W|PG_P
    mov [kern_page_table],ebx
    
    ;修改最后一个页目录项   映射到页目录起始地址
    .change_last_pde
    mov eax,kern_dir_table
    or eax,PG_US_S|PG_RW_W|PG_P    
    mov [kern_dir_table+0xc00+255*4],eax

    ret

section .init.data align=4096            ;依据我的测试结果来看   section是默认4096对齐的
;需要4K对齐
;内核页目录表
kern_dir_table:
 resb 4096
 ;内核页表 起始位置  一共256个页表
kern_page_table:
resb 4096*256          
init_stack:
resb 1024      ;1024B暂用栈
INIT_STACK_TOP equ $-1
[BITS 32]   ;由于GRUB在加载内核前进入保护模式，所以要32位编译   
section .text    
[EXTERN kern_entry]
   GDT_BASE:   dd    0x00000000 
           	   dd    0x00000000

   CODE_DESC:  dd    0x0000FFFF 
               dd    DESC_CODE_HIGH4

   DATA_STACK_DESC:  dd    0x0000FFFF
                     dd    DESC_DATA_HIGH4

   VIDEO_DESC: dd    0x80000007        ; limit=(0xbffff-0xb8000)/4k=0x7
               dd    DESC_VIDEO_HIGH4  ; 此时dpl为0

    ;---------新增段描述符-----------
    ;用户代码段与数据段
    USER_CODE_DESC: dd 0x0000FFFF
                dd   DESC_USER_CODE_HIGH4

    USER_DATA_DESC: dd 0x0000FFFF
                dd   DESC_USER_DATA_HIGH4
;------------------------------------------


   GDT_SIZE   equ   $ - GDT_BASE
   GDT_LIMIT   equ   GDT_SIZE - 1 
   SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0     ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
   SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0     ; 同上
   SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0    ; 同上 
   ;SELECTOR_USER_CODE equ (0x004<<3) + TI_GDT + RPL3
   ;SELECTOR_USER_DATA equ (0x005<<3) + TI_GDT + RPL3
   total_mem_bytes dd 0                  
   ;以下是定义gdt的指针，前2字节是gdt界限，后4字节是gdt起始地址
   gdt_ptr  dw  GDT_LIMIT 
        	dd  GDT_BASE
;boot开始！
boot_start_after_set_paging:        ;此处修改了函数名     在设置好页表后调用此函数
    mov ebx,[temp_mboot_ptr]     ;此处将暂存的mboot信息取出    但是一定要注意：必须要前4MB的物理-虚拟内存映射才能够使用
    mov [mboot_ptr], ebx ; GRUB加载内核后会将mutiboot信息地址存放在ebx中
    ;-----------------   准备进入保护模式   -------------------
;1 打开A20
;2 加载gdt
;3 将cr0的pe位置1
   ;-----------------  打开A20  ----------------
    in al,0x92
    or al,0000_0010B
    out 0x92,al
   ;-----------------  加载GDT  ----------------
    lgdt [gdt_ptr]
   ;-----------------  cr0第0位置1  ----------------
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax
    jmp dword SELECTOR_CODE:far_jmp_target      ; 刷新流水线，避免分支预测的影响,这种cpu优化策略，最怕jmp跳转，

;初始化段寄存器以及栈结构
    far_jmp_target:
    mov ax,SELECTOR_DATA
    mov ss,ax
    mov ds,ax
  	mov ax,SELECTOR_VIDEO
  	mov gs,ax
    mov esp, STACK_TOP      
    and esp, 0xFFFFFFF0  ;16字节对齐
    mov ebp, 0         
    mov eax,kern_bitmap_block
    mov [kern_bitmap],eax
    mov eax,kern_dir_table
    mov [kern_dir_table_paddr],eax
    mov eax,kern_page_table
    mov [kern_page_table_paddr],eax
;进入内核主函数    
    cli
    call kern_entry                    
    jmp dword $          ;防止意外退出内核

section .data
[GLOBAL mboot_ptr]  
[GLOBAL kern_bitmap]
[GLOBAL kern_dir_table_paddr]
[GLOBAL kern_page_table_paddr]
kern_bitmap:
    dd 0x0
mboot_ptr:        
    dd 0x0        

kern_dir_table_paddr:
    dd 0x0
kern_page_table_paddr:
    dd 0x0

section .bss             ; 未初始化的数据段从这里开始    注意bss段是不占用存储器空间的，是在程序加载后才在内存中分配的

kern_bitmap_block:
    ;内核空间1GB   需要1024*1024*1KB=1024*256*4KB，所以一共有1024*256bit = 1024*32B=32KB
    dd 0x0
    resb  0x8000;32*1024   
stack:
    resb 0x80000        ; 512KB的内核栈 (应该够了吧,不够自己改)
STACK_TOP equ $-1      