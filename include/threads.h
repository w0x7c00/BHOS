#ifndef THREADS_H
#define THREADS_H
#include "types.h"
typedef 			//定义线程或进程状态
enum task_status_t{     
	TASK_RUNNING,
	TASK_READY,
	TASK_BLOCKED,
	TASK_WAITING,
	TASK_HANGING,
	TASK_DIED
} task_status_t;

typedef
struct context_t{       //存放在内核栈中的任务上下文
    uint32_t ebp;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
	uint32_t esp;     //esp是保存在kern_stack_top中的       
} __attribute__((packed)) context_t;       //由于要在汇编中使用 要编译成连续的分布

typedef
struct TCB_t{
	uint32_t * kern_stack_top;    //对应的内核栈顶地址
	task_status_t task_status;  
	uint32_t time_counter;      //记录运行总的时钟中断数
	uint32_t time_left;         //剩余时间片
	struct TCB_t * next;        //下一个TCB(用于线程调度)
	//uint32_t idt_addr;        在用户进程中使用的页表 内核线程可以不加入此字段
	uint32_t tid;               //线程id
	uint32_t page_counte;       //分配的页空间大小
	uint32_t page_addr;			//page_counte与page_addr用于释放内存
	context_t context;
} TCB_t;

typedef void * thread_function(void * args);       //定义线程的实际执行函数类型

extern void switch_to(void * cur_coontext_ptr,void * next_context_ptr);    //使用汇编完成的切换上下文函数

extern uint32_t get_esp();

void schedule();

void create_thread(uint32_t tid,thread_function *func,void * args,uint32_t addr,uint32_t page_counte); //创建线程函数

void threads_init();    //线程模块初始化 需要把主线程加入运行表中

void exit();     //线程结束函数 关闭中断->移出执行链表->回收内存空间->开启中断
#endif