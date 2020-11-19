#include "threads.h"
#include "types.h"
#include "printk.h"
#include "kern_log.h"
#include "vmm.h"
#include "user_task.h"
#include "interrupt.h"
#define TIME_CONT  10 //默认时间片计数
TCB_t main_TCB;    //内核主线程TCB
TCB_t* cur_tcb;
static char* LOG_SRC_THREADS = "THREADS";


TCB_t* get_running_progress(){
	return cur_tcb;
}

bool check_kern_stack_overflow(TCB_t* tcb_ptr){
	if(tcb_ptr->tcb_magic_number==TCB_MAGIC_NUMBER){
		return True;
	}
	else{
		return False;
	}	
}

static void kern_overflow_handler(TCB_t * tcb_ptr){
	uint32_t tid = tcb_ptr->tid;
	STOP(LOG_SRC_THREADS,"Thread Kern Stack Overflow!STOP!");
}

//线程调度的第一步
//主要功能： 1 页表切换   2  tss栈修改    
// static void active_task(TCB_t * next){
	
// }

static void  _init_main_thread(TCB_t * main_tcb){
	TCB_t *tcb_buffer_addr = main_tcb;
	tcb_buffer_addr->tid = 0;        //主线程的编号为0  
	tcb_buffer_addr->time_counter=0;
	tcb_buffer_addr->time_left=TIME_CONT;
	tcb_buffer_addr->task_status = TASK_RUNNING;
	tcb_buffer_addr->page_counte=0;   //主线程不会被回收内存 所以可以任意赋值
	tcb_buffer_addr->page_addr=0;
	tcb_buffer_addr->next = tcb_buffer_addr;
	tcb_buffer_addr->kern_stack_top=0;
	tcb_buffer_addr->is_kern_thread = True;
	tcb_buffer_addr->tcb_magic_number = TCB_MAGIC_NUMBER;
}

void threads_init(){
	TCB_t *tcb_buffer_addr = &main_TCB;
	_init_main_thread(&main_TCB);
	cur_tcb = &main_TCB;
}

//用于创建线程的PCB
TCB_t* create_TCB(uint32_t tid,uint32_t page_addr,uint32_t page_counte){
	TCB_t * tcb_buffer_addr = (TCB_t*)page_addr;
	tcb_buffer_addr->tid = tid;         
	tcb_buffer_addr->time_counter=0;
	tcb_buffer_addr->time_left=TIME_CONT;
	tcb_buffer_addr->task_status = TASK_READY;
	tcb_buffer_addr->page_counte=page_counte; 
	tcb_buffer_addr->page_addr=page_addr;
	tcb_buffer_addr->kern_stack_top=page_addr+page_counte*PAGE_SIZE;    
	tcb_buffer_addr->tcb_magic_number = TCB_MAGIC_NUMBER;
	return (TCB_t*)page_addr;
}

//创建最终线程的核心函数     创建用户进程以及创建内核线程的函数都是对这个函数的封装
//会操作TCB链表 需要加锁
void create_thread(uint32_t tid,thread_function *func,void *args,uint32_t addr,uint32_t page_counte,bool is_kern_thread,bitmap user_vmm_pool,uint32_t pdt_vaddr){	
	asm volatile("cli");  //由于创建过程会使用到共享的数据 不使用锁的话会造成临界区错误 所以我们在此处关闭中断
	TCB_t * new_tcb = create_TCB(tid,addr,page_counte);
	TCB_t * temp_next = cur_tcb->next;
	cur_tcb->next = new_tcb;
	new_tcb->next = temp_next;
	new_tcb->is_kern_thread = is_kern_thread;
	if(!is_kern_thread){
		//用户进程需要填充页表等
		new_tcb->user_vmm_pool = user_vmm_pool;
		new_tcb->pdt_vaddr = pdt_vaddr;
	}
	*(--new_tcb->kern_stack_top)=args;     //压入初始化的参数与线程执行函数
	*(--new_tcb->kern_stack_top)=exit;
	*(--new_tcb->kern_stack_top)=func;
	//此处存在修改！    0x200 ------->0x202    IF为1（打开硬中断）   IOPL为0（只允许内核访问IO）   1号位为1（eflags格式默认）
	new_tcb->context.eflags = 0x202; 
	new_tcb->context.esp =new_tcb->kern_stack_top;
	asm volatile("sti");	
}


//内核线程必须要保证两点：
//                                            1.func必须保证存在，不会被内存回收
//                                            2.args必须保证存在， 不会被内存回收
//内核线程的func与args都是内核内存空间中的     func通过函数定义的方式保存在os内核的程序段中 ，args保存在调用者函数的定义中，一旦调用者函数退出，args就会被回收
//如何解决这个问题？  线程的创建者函数不能退出！！！使用特定指令阻塞对应的函数（join）
//使用detach，在detach中实现线程将参数复制
void create_kern_thread(uint32_t tid,thread_function *func,void *args){
	bitmap default_bitmap;
	uint32_t page_counte = 1;
	uint32_t TCB_page = vmm_kern_alloc();
	uint32_t default_pdt_vaddr = 0x0;
	bool is_kern_thread = True;
    if(TCB_page==KERN_VMM_ALLOC_ERRO){
        STOP(LOG_SRC_THREADS,"Can`t Create New User Task Because Of Error When Alloc TCB Page From Kernel VMM!STOP!");
    }
	create_thread(tid,func,args,TCB_page,page_counte,is_kern_thread,default_bitmap,default_pdt_vaddr);
}

void schedule(){
    //check if the thread module is available
    if(get_running_progress()==NULL){
        return ;
    }
    //调度函数  检测时间片为0时调用此函数
	//首先判定现在的线程内核栈是否溢出
	if(!check_kern_stack_overflow(get_running_progress())){
		//溢出处理！！！
		kern_overflow_handler(get_running_progress());
	}

	//find next thread
    TCB_t * probe = cur_tcb;
    while(True){
        probe = probe->next;
        if(probe==cur_tcb){
            if(probe->task_status!=TASK_READY&&probe->task_status!=TASK_RUNNING){
                // all of the threads are blocked,which is not allowed
                //walk up cur thread
                WARNING(LOG_SRC_THREADS,"ALL OF THE THREADS ARE BLOCKED,SELF WAKE UP ONE!WARNNING!");
                cur_tcb->time_left = TIME_CONT;    //如果只有一个线程 就再次给此线程添加时间片
                cur_tcb->task_status = TASK_RUNNING;
                return ;
            }
            else{
                //only cur thread can run
                cur_tcb->time_left =TIME_CONT;
                return ;
            }
        }
        else{
            if(probe->task_status== TASK_READY){
                break;
            }
            else{
                continue;
            }
        }
    }
	//进行调度
	TCB_t *now = cur_tcb;
	TCB_t *next_tcb = probe;

	//if cur task is blocked,it will not be set ready or cur thread will running next schedule
	if(now->task_status == TASK_RUNNING){
	    now->task_status = TASK_READY;
	}
	next_tcb->task_status = TASK_RUNNING;
	next_tcb->time_left = TIME_CONT;
	cur_tcb = next_tcb;
	active_task(cur_tcb);
	switch_to(&(now->context),&(next_tcb->context));      
}

//block running threads , which must be called in kernel state(level 0)
//however,when the task is in user state(level 3),can use
//syscall to jump in kernel state, then invoke the function

void thread_block(){
    //cli and sti is used to sync to access the threads list and it`s node information
    TCB_t* now = get_running_progress();
    bool condition=cli_condition();
    now->task_status = TASK_BLOCKED;
    schedule();
    //reload the interrupt flag before block
    sti_condition(condition);
}

void thread_wakeup(TCB_t * target_thread){
    enum task_status_t restore_status = get_running_progress()->task_status;
    bool  condition = cli_condition();
    target_thread->task_status = TASK_READY;
    sti_condition(condition);
}

void remove_thread(){
	cli();
	if(cur_tcb->tid==0)
		printk("ERRO:main thread can`t use function exit\n");
	else{
		TCB_t *temp = cur_tcb;
		for(;temp->next!=cur_tcb;temp=temp->next)
			;
		temp->next = cur_tcb->next;
	}
}


//本exit函数暂时只能被内核线程使用 作为自动退出
void exit(){
	remove_thread();
	TCB_t *now = cur_tcb;
	TCB_t *next_tcb = cur_tcb->next;
	next_tcb->time_left = TIME_CONT;
	cur_tcb = cur_tcb->next;
	switch_to(&(now->context),&(next_tcb->context));
	//注意 暂时没有回收此线程页
}

#undef TIME_CONT