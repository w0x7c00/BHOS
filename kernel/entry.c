#include "types.h"
#include "vga_basic.h"
#include "printk.h"
#include "init.h"
#include "pmm.h"
#include "threads.h"
#include "bitmap.h"
#include "vmm.h"
void clear_screen();
void kputc(char);
void screen_uproll_once();
uint32_t get_eflags();
extern TCB_t * cur_tcb;
extern TCB_t main_TCB;
void kern_entry(){
	void func(void* args);
	vga_init();
	pmm_init();
	idt_init();
	printk("0x%h\n\n",0xC0100000);
	vmm_init();
	while(True){
	}
	// pm_alloc_t re = pmm_alloc_pages(1);
	// printk("addr:0x%h,size:%d,state:%d\n",re.addr,re.size,(int)re.state);
	// pmm_show_page_count();
	threads_init();
	//pmm需要在关中断的时候使用
	create_thread(1,(thread_function *)func,0,pmm_alloc_one_page().addr,1);
    asm volatile ("sti");   //要在主线程加载完后开中断
	while(True){
		asm volatile("cli");
		printk("A");
		asm volatile("sti");
	}

    while(True)
    	asm volatile ("hlt");
}

void func(void* args){
	while(True){
		asm volatile("cli");
		printk_color("B",15,0);
		asm volatile("sti");
	}
}
