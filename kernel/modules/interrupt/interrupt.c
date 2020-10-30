#include "interrupt.h"
#include "_8259A.h"
#include "types.h"
#include "printk.h"
#include "threads.h"

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

static void registe_interrupt(int int_no,int_server_func_t target_func){
	int_server_func_list[int_no] = target_func;
}

void default_server_func(void *args){
	//printk("Default Int server function!\n");
}


extern TCB_t * cur_tcb; 

//时钟中断函数 主要用于线程调度
void timer_server_func(void *args){
	if(cur_tcb->time_left!=0){
		(cur_tcb->time_left)--;
		(cur_tcb->time_counter)++;
	}
	else{
		schedule();
	}
}

//cr2 保存引起缺页的线性地址
void get_cr2();
extern uint32_t _CR2;
void page_fault_func(void * args){
	get_cr2();
	printk("INT 14:Page Fault---0x%h\n",_CR2);
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
	registe_interrupt(0,default_server_func);
	registe_interrupt(1,default_server_func);
	registe_interrupt(2,default_server_func);
	registe_interrupt(3,default_server_func);
	registe_interrupt(4,default_server_func);
	registe_interrupt(5,default_server_func);
	registe_interrupt(6,default_server_func);
	registe_interrupt(7,default_server_func);
	registe_interrupt(8,default_server_func);
	registe_interrupt(9,default_server_func);
	registe_interrupt(10,default_server_func);
	registe_interrupt(11,default_server_func);
	registe_interrupt(12,default_server_func);
	registe_interrupt(13,default_server_func);
	registe_interrupt(14,default_server_func);
	registe_interrupt(15,default_server_func);
	registe_interrupt(16,default_server_func);
	registe_interrupt(17,default_server_func);
	registe_interrupt(18,default_server_func);
	registe_interrupt(19,default_server_func);
	registe_interrupt(20,default_server_func);
	registe_interrupt(21,default_server_func);
	registe_interrupt(22,default_server_func);
	registe_interrupt(23,default_server_func);
	registe_interrupt(24,default_server_func);
	registe_interrupt(25,default_server_func);
	registe_interrupt(26,default_server_func);
	registe_interrupt(27,default_server_func);
	registe_interrupt(28,default_server_func);
	registe_interrupt(29,default_server_func);
	registe_interrupt(30,default_server_func);
	registe_interrupt(31,default_server_func);
	//registe_interrupt(32,default_server_func);
	
	registe_interrupt(14,page_fault_func);
	registe_interrupt(32,timer_server_func);
	lidt_target.limit = sizeof(interrupt_discripter_t)*256;
	lidt_target.base = (uint32_t)&idt_entries;
	timer_init(1000);
	load_idt((uint32_t)&lidt_target);
    sti();
}

// INT路由函数
void int_func_route(int int_no,void * args){
	int_server_func_list[int_no](args);
}

void cli(){
    asm volatile("cli");
}
void sti(){
    asm volatile("sti");
}