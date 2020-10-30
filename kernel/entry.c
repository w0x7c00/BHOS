#include "types.h"
#include "vga_basic.h"
#include "printk.h"
#include "init.h"
#include "pmm.h"
#include "threads.h"
#include "bitmap.h"
#include "vmm.h"
#include "user_task.h"
#include "kern_log.h"
#include "tss.h"
void clear_screen();
void kputc(char);
void screen_uproll_once();
extern TCB_t * cur_tcb;
extern TCB_t main_TCB;
int entry_test_a;
void kern_entry(){
	void func(void* args);
	vga_init();
	pmm_init();
	printk("kern_pdt_paddr:0x%h\n",kern_dir_table_paddr);
	tss_init();
	printk("init\n");
  while (1);
	idt_init();
    //must close hardware interrupt because we have just user IRQ0(Number32/clock)
//asm volatile("sti");
  
    //vga_basic_test();
	vmm_init();
    vmm_test();
    //bitmap_test();
	user_task_test();

while (1){
		asm volatile("hlt");
	}

	// pm_alloc_t re = pmm_alloc_pages(1);
	// printk("addr:0x%h,size:%d,state:%d\n",re.addr,re.size,(int)re.state);
	// pmm_show_page_count();
	threads_init();
	//pmm需要在关中断的时候使用
	
	//create_thread(1,(thread_function *)func,0,vmm_kern_alloc(),1);
    
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