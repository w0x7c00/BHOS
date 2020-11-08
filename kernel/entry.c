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
#include "interrupt.h"
void kern_entry(){
    void func(void* args);
	vga_init();
	pmm_init();
    //vmm pre init must be invoked after pmm init,
    // because pmm init will use mboot_struct
    //which is located in virtual Mem below first 4MB.
	vmm_pre_init();
    vmm_init();
    idt_init();
    tss_init();
    //careful!Close hardware interrupt because of that
    // we have just use IRQ0(Number32/for clock)
	threads_init();
	//open hardware interrupt
	sti();
	//create user task for test.
	start_user_task_params_t u1;
	u1.fd = 0;
	u1.is_from_file = False;
	u1.function = func;
	u1.args = (void*)NULL;
	//create_user_task(2,&u1);
	create_kern_thread(1,func,NULL);
	thread_block();
    while (1) {
    //main threads loop
    //do nothing
    }
}

void func(void* args){
    TCB_t * probe =get_running_progress();
    for(;probe->tid!=0;probe=probe->next){
    }
    thread_wakeup(probe);
	while(True){
        bool condition =cli_condition();
	    //printk("TASK1\n");
        sti_condition(condition);
	}
}