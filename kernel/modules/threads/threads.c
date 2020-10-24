#include "threads.h"
#include "types.h"
#include "pmm.h"
#include "printk.h"
#define TIME_CONT  2 //默认时间片计数
TCB_t main_TCB;    //内核主线程TCB
TCB_t* cur_tcb;


TCB_t* get_running_progress(){
	return cur_tcb;
}
void threads_init(){
	TCB_t *tcb_buffer_addr = &main_TCB;
	tcb_buffer_addr->tid = 0;        //主线程的编号为0  
	tcb_buffer_addr->time_counter=0;
	tcb_buffer_addr->time_left=TIME_CONT;
	tcb_buffer_addr->task_status = TASK_RUNNING;
	tcb_buffer_addr->page_counte=0;   //主线程不会被回收内存 所以可以任意赋值
	tcb_buffer_addr->page_addr=0;
	tcb_buffer_addr->next = tcb_buffer_addr;
	tcb_buffer_addr->kern_stack_top=0;
	cur_tcb = tcb_buffer_addr;
}

uint32_t create_TCB(uint32_t tid,uint32_t page_addr,uint32_t page_counte){
	TCB_t * tcb_buffer_addr = (TCB_t*)page_addr;
	tcb_buffer_addr->tid = tid;         
	tcb_buffer_addr->time_counter=0;
	tcb_buffer_addr->time_left=TIME_CONT;
	tcb_buffer_addr->task_status = TASK_RUNNING;
	tcb_buffer_addr->page_counte=page_counte; 
	tcb_buffer_addr->page_addr=page_addr;
	tcb_buffer_addr->kern_stack_top=page_addr+page_counte*4096;
	return page_addr;
}

void create_thread(uint32_t tid,thread_function *func,void *args,uint32_t addr,uint32_t page_counte){	
	asm volatile("cli");  //由于创建过程会使用到共享的数据 不使用锁的话会造成临界区错误 所以我们在此处关闭中断
	TCB_t * new_tcb = create_TCB(tid,addr,page_counte);
	TCB_t * temp_next = cur_tcb->next;
	cur_tcb->next = new_tcb;
	new_tcb->next = temp_next;
	*(--new_tcb->kern_stack_top)=args;     //压入初始化的参数与线程执行函数
	*(--new_tcb->kern_stack_top)=exit;
	*(--new_tcb->kern_stack_top)=func;
	new_tcb->context.eflags = 0x200;
	new_tcb->context.esp =new_tcb->kern_stack_top;
	asm volatile("sti");	
}

void schedule(){      //调度函数  检测时间片为0时调用此函数
	if(cur_tcb->next==cur_tcb){
		cur_tcb->time_left = TIME_CONT;    //如果只有一个线程 就再次给此线程添加时间片
		return ;
	}
	TCB_t *now = cur_tcb;
	TCB_t *next_tcb = cur_tcb->next;
	next_tcb->time_left = TIME_CONT;
	cur_tcb = next_tcb;
	get_esp();      //有一个隐藏bug 需要call刷新寄存器
	switch_to(&(now->context),&(next_tcb->context));      
}

void remove_thread(){
	asm volatile("cli");
	if(cur_tcb->tid==0)
		printk("ERRO:main thread can`t use function exit\n");
	else{
		TCB_t *temp = cur_tcb;
		for(;temp->next!=cur_tcb;temp=temp->next)
			;
		temp->next = cur_tcb->next;
	}
}

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