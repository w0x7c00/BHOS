#include "interrupt.h"
#include "_8259A.h"
#include "types.h"
#include "printk.h"
#include "threads.h"
#include "common_asm.h"

int_server_func_t int_server_func_list[256];
interrupt_discripter_t idt_entries[256];    //中断描述符表 idt_entries为表首指针
lidt_target_t lidt_target;
static uint8_t default_inf=0x8E;
static uint16_t kern_cs=0x08; 
static void set_int_disc(int int_no, uint32_t offset, uint16_t selector, uint8_t inf){
	idt_entries[int_no].offset_low=(uint16_t)offset;
	idt_entries[int_no].offset_high=(uint16_t)(offset>>16);
	idt_entries[int_no].selector=selector;
	idt_entries[int_no].inf=inf;
	idt_entries[int_no].default_bit8=0;
}
static void timer_init(uint32_t frequency){
	// Intel 8253/8254 PIT芯片 I/O端口地址范围是40h~43h
    // 输入频率为 1193180，frequency 即每秒中断次数
    uint32_t divisor = 1193180 / frequency;
    // D7 D6 D5 D4 D3 D2 D1 D0
    // 0  0  1  1  0  1  1  0
    // 即就是 36 H
    // 设置 8253/8254 芯片工作在模式 3 下
    outb(0x43, 0x36);

    // 拆分低字节和高字节
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t hign = (uint8_t)((divisor >> 8) & 0xFF);
    
    // 分别写入低字节和高字节
    outb(0x40, low);
    outb(0x40, hign);
}

//int handler list is a shared source , must change the value with lock
void register_interrupt(int int_no, int_server_func_t target_func){
	int_server_func_list[int_no] = target_func;
}

void default_server_func(uint32_t* int_no,void *args){
	printk("INT %d Default Int server function!\n",int_no);
}


extern TCB_t * cur_tcb; 

//时钟中断函数 主要用于线程调度
void timer_server_func(uint32_t int_no,void *args){
    if(cur_tcb->time_left!=0){
		(cur_tcb->time_left)--;
		(cur_tcb->time_counter)++;
	}
	else{
        //printk("Schedule!Running Task ID : %d,Kernel Task:%d\n",get_running_progress()->tid,get_running_progress()->is_kern_thread);
		schedule();
	}
}

//cr2 保存引起缺页的线性地址
void get_cr2();
extern uint32_t _CR2;
void page_fault_func(uint32_t int_no,void * args){
	get_cr2();
	struct TCB_t* cur_tcb=get_running_progress();
    if(cur_tcb==NULL){
        printk("INT 14:Page Fault:0x%h IN before init threads\n",_CR2);
    }
	else{
	    printk("INT 14:Page Fault:0x%h IN TASK:%d\n",_CR2,cur_tcb->tid);
	}
}

extern void load_idt(uint32_t);     //声明idt装载使用的函数(定义在interrupt_asm.s中)
//要注意手动发送EOI
void isr0();
void isr1();
void isr2();
void isr3();
void isr4();
void isr5();
void isr6();
void isr7();
void isr8();
void isr9();
void isr10();
void isr11();
void isr12();
void isr13();
void isr14();
void isr15();
void isr16();
void isr17();
void isr18();
void isr19();
void isr20();
void isr21();
void isr22();
void isr23();
void isr24();
void isr25();
void isr26();
void isr27();
void isr28();
void isr29();
void isr30();
void isr31();

void isr32();
void isr33();
void isr34();
void isr35();
void isr36();
void isr37();
void isr38();
void isr39();
void isr40();
void isr41();
void isr42();
void isr43();
void isr44();
void isr45();
void isr46();
void isr47();

void idt_init(){
	_8259A_init();   //初始化中断控制器
	set_int_disc(0,(uint32_t)isr0,kern_cs,default_inf);
	set_int_disc(1,(uint32_t)isr1,kern_cs,default_inf);
	set_int_disc(2,(uint32_t)isr2,kern_cs,default_inf);
	set_int_disc(3,(uint32_t)isr3,kern_cs,default_inf);
	set_int_disc(4,(uint32_t)isr4,kern_cs,default_inf);
	set_int_disc(5,(uint32_t)isr5,kern_cs,default_inf);
	set_int_disc(6,(uint32_t)isr6,kern_cs,default_inf);
	set_int_disc(7,(uint32_t)isr7,kern_cs,default_inf);
	set_int_disc(8,(uint32_t)isr8,kern_cs,default_inf);
	set_int_disc(9,(uint32_t)isr9,kern_cs,default_inf);
	set_int_disc(10,(uint32_t)isr10,kern_cs,default_inf);
	set_int_disc(11,(uint32_t)isr11,kern_cs,default_inf);
	set_int_disc(12,(uint32_t)isr12,kern_cs,default_inf);
	set_int_disc(13,(uint32_t)isr13,kern_cs,default_inf);
	set_int_disc(14,(uint32_t)isr14,kern_cs,default_inf);
	set_int_disc(15,(uint32_t)isr15,kern_cs,default_inf);
	set_int_disc(16,(uint32_t)isr16,kern_cs,default_inf);
	set_int_disc(17,(uint32_t)isr17,kern_cs,default_inf);
	set_int_disc(18,(uint32_t)isr18,kern_cs,default_inf);
	set_int_disc(19,(uint32_t)isr19,kern_cs,default_inf);
	set_int_disc(20,(uint32_t)isr20,kern_cs,default_inf);
	set_int_disc(21,(uint32_t)isr21,kern_cs,default_inf);
	set_int_disc(22,(uint32_t)isr22,kern_cs,default_inf);
	set_int_disc(23,(uint32_t)isr23,kern_cs,default_inf);
	set_int_disc(24,(uint32_t)isr24,kern_cs,default_inf);
	set_int_disc(25,(uint32_t)isr25,kern_cs,default_inf);
	set_int_disc(26,(uint32_t)isr26,kern_cs,default_inf);
	set_int_disc(27,(uint32_t)isr27,kern_cs,default_inf);
	set_int_disc(28,(uint32_t)isr28,kern_cs,default_inf);
	set_int_disc(29,(uint32_t)isr29,kern_cs,default_inf);
	set_int_disc(30,(uint32_t)isr30,kern_cs,default_inf);
	set_int_disc(31,(uint32_t)isr31,kern_cs,default_inf);
	set_int_disc(32,(uint32_t)isr32,kern_cs,default_inf);
    set_int_disc(33,(uint32_t)isr33,kern_cs,default_inf);
    set_int_disc(34,(uint32_t)isr34,kern_cs,default_inf);
    set_int_disc(35,(uint32_t)isr35,kern_cs,default_inf);
    set_int_disc(36,(uint32_t)isr36,kern_cs,default_inf);
    set_int_disc(37,(uint32_t)isr37,kern_cs,default_inf);
    set_int_disc(38,(uint32_t)isr38,kern_cs,default_inf);
    set_int_disc(39,(uint32_t)isr39,kern_cs,default_inf);
    set_int_disc(40,(uint32_t)isr40,kern_cs,default_inf);
    set_int_disc(41,(uint32_t)isr41,kern_cs,default_inf);
    set_int_disc(42,(uint32_t)isr42,kern_cs,default_inf);
    set_int_disc(43,(uint32_t)isr43,kern_cs,default_inf);
    set_int_disc(44,(uint32_t)isr44,kern_cs,default_inf);
    set_int_disc(45,(uint32_t)isr45,kern_cs,default_inf);
    set_int_disc(46,(uint32_t)isr46,kern_cs,default_inf);
    set_int_disc(47,(uint32_t)isr47,kern_cs,default_inf);


    register_interrupt(0, default_server_func);
    register_interrupt(1, default_server_func);
    register_interrupt(2, default_server_func);
    register_interrupt(3, default_server_func);
    register_interrupt(4, default_server_func);
    register_interrupt(5, default_server_func);
    register_interrupt(6, default_server_func);
    register_interrupt(7, default_server_func);
    register_interrupt(8, default_server_func);
    register_interrupt(9, default_server_func);
    register_interrupt(10, default_server_func);
    register_interrupt(11, default_server_func);
    register_interrupt(12, default_server_func);
    register_interrupt(13, default_server_func);
    register_interrupt(14, default_server_func);
    register_interrupt(15, default_server_func);
    register_interrupt(16, default_server_func);
    register_interrupt(17, default_server_func);
    register_interrupt(18, default_server_func);
    register_interrupt(19, default_server_func);
    register_interrupt(20, default_server_func);
    register_interrupt(21, default_server_func);
    register_interrupt(22, default_server_func);
    register_interrupt(23, default_server_func);
    register_interrupt(24, default_server_func);
    register_interrupt(25, default_server_func);
    register_interrupt(26, default_server_func);
    register_interrupt(27, default_server_func);
    register_interrupt(28, default_server_func);
    register_interrupt(29, default_server_func);
    register_interrupt(30, default_server_func);
    register_interrupt(31, default_server_func);
	//register_interrupt(32,default_server_func);

    register_interrupt(33, default_server_func);
    register_interrupt(34, default_server_func);
    register_interrupt(35, default_server_func);
    register_interrupt(36, default_server_func);
    register_interrupt(37, default_server_func);
    register_interrupt(38, default_server_func);
    register_interrupt(39, default_server_func);
    register_interrupt(40, default_server_func);
    register_interrupt(41, default_server_func);
    register_interrupt(42, default_server_func);
    register_interrupt(43, default_server_func);
    register_interrupt(44, default_server_func);
    register_interrupt(45, default_server_func);
    register_interrupt(46, default_server_func);
    register_interrupt(47, default_server_func);


    register_interrupt(14, page_fault_func);
    register_interrupt(32, timer_server_func);
	lidt_target.limit = sizeof(interrupt_discripter_t)*256;
	lidt_target.base = (uint32_t)&idt_entries;
	timer_init(1000);
	load_idt((uint32_t)&lidt_target);
    //can`t open hardware interrupt before threads module init
	cli();
}

// INT路由函数
void int_func_route(int int_no,void * args){
	int_server_func_list[int_no](int_no,args);
}

void cli(){
    asm volatile("cli");
}
void sti(){
    asm volatile("sti");
}


static bool get_int_flag(){
    uint32_t eflag_mem=NULL;
    uint32_t * eflag = &eflag_mem;
    get_eflag(eflag);
    if(((eflag_mem>>9)&0x00000001) == 1){
        return True;
    }
    else{
        return False;
    }
}

bool cli_condition(){
    if(get_int_flag()){
        cli();
        return True;
    }
    else{
        return False;
    }
}

void sti_condition(bool condition){
    if(condition){
        sti();
    }
    else{
        cli();
    }
}