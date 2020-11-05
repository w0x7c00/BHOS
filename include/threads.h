#ifndef THREADS_H
#define THREADS_H
#include "types.h"
#include "bitmap.h"
#define TCB_MAGIC_NUMBER  0xFC0D21AB

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

//TCB结构 对于用户进程与内核线程 TCB结构是相同的 不同之处在于：
//

typedef
struct TCB_t{
	uint32_t * kern_stack_top;    //对应的内核栈顶地址
	task_status_t task_status;  
	uint32_t time_counter;      //记录运行总的时钟中断数
	uint32_t time_left;         //剩余时间片
	struct TCB_t * next;        //下一个TCB(用于线程调度)
	//uint32_t idt_addr;        在用户进程中使用的页表 内核线程可以不加入此字段
	uint32_t tid;               //线程id
	uint32_t page_counte;       //TCB分配的内核页空间大小  越大的空间 其内核栈大小越大
	uint32_t page_addr;			//page_counte与page_addr用于释放内存


	bool is_kern_thread;         //识别是否为内核线程     （如果为内核线程的话就不进行换页等处理步骤）
	bitmap user_vmm_pool;    //定义虚拟内存池结构
	uint32_t pdt_vaddr;    //定义进程虚拟页表在内核态中的虚拟地址（使用内核页表）    由于分配在内核内存中 所以pdt是只能在内核中进行读写操作的

	context_t context;

	uint32_t tcb_magic_number;    //这个32位的magic number用来检查内核栈是否溢出
} TCB_t;

typedef void * thread_function(void * args);       //定义线程的实际执行函数类型

bool check_kern_stack_overflow(TCB_t* tcb_ptr);    

extern void switch_to(void * cur_context_ptr,void * next_context_ptr);    //使用汇编完成的切换上下文函数

extern uint32_t get_esp();

void schedule();

void create_thread(uint32_t tid,thread_function *func,void *args,uint32_t addr,uint32_t page_counte,bool is_kern_thread,bitmap user_vmm_pool,uint32_t pdt_vaddr);

void create_kern_thread(uint32_t tid,thread_function *func,void *args);

void threads_init();    //线程模块初始化 需要把主线程加入运行表中

TCB_t* create_TCB(uint32_t tid,uint32_t page_addr,uint32_t page_counte);

TCB_t* get_running_progress();

void exit();     //线程结束函数 关闭中断->移出执行链表->回收内存空间->开启中断
#endif