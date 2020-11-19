#include "types.h"
#include "vga_basic.h"
#include "printk.h"
#include "init.h"
#include "pmm.h"
#include "threads.h"
#include "vmm.h"
#include "user_task.h"
#include "tss.h"
#include "interrupt.h"
#include "sync.h"

#include "keyboard.h"
lock_t lock;
sem_t sem;
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
    printk_init_lock();
    keyboard_init();
	//open hardware interrupt
	sti();
	//create user task for test.
	start_user_task_params_t u1;
	u1.fd = 0;
	u1.is_from_file = False;
	u1.function = func;
	u1.args = (void*)NULL;
	lock_init(&lock);
	sem_init(&sem,2);
	//create_user_task(2,&u1);
	create_kern_thread(1,func,NULL);

    uint32_t  func_addr=(uint32_t)func;

    printk("0x%h\n",func_addr);
    while (1);
    for(int i = 0;i>=0;i++) {
        printk("TASK1\n");
    }
    while (1);
}

void func(void* args){
    while (1);

    for(int i = 0;i>=0;i++) {
        printk("TASK2\n");
    }
    while(True){

    }
}