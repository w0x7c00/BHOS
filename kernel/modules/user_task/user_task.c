/*
by 不吃香菜的大头怪     GitHub：CQU-code-lover
2020-10-24
描述： 本文件包含用于创建/回收用户进程相关函数
*/
#include "user_task.h"
#include "bitmap.h"
#include "vmm.h"
#include "printk.h"
#include "string.h"
#include "threads.h"
#include "kern_log.h"
#include "interrupt.h"
#include "tss.h"
static char* LOG_SRC_USER_TASK = "USER_TASK";

//创建用户专用bitmap
//用户的bitmap会占用内核虚拟空间 要在内核空间池中分配 
//注意 进程结束时一定要回收这部分空间
//the bitmap have been init before return;
bitmap create_user_task_bitmap(){
    //ask <96KB> for Bitmap!!!
    //alloc kern vm for bitmap
    //get the first 4KB
    uint32_t val_4KB = 0x1000;
    uint32_t vm_head = vmm_kern_alloc();
    if(vm_head==KERN_VMM_ALLOC_ERRO){
        printk("[ERRO]:user_task.c/0");
        while(True);
    }
// for 96KB/4KB-1=23 times
    bool alloc_flag = True;
    uint32_t addr_for = vm_head+val_4KB;
    for(int item=0;item<23;item++){
        if(vmm_kern_alloc_one_page(addr_for)==KERN_VMM_ALLOC_ERRO){
            alloc_flag = False;
            break;
        }
        addr_for += val_4KB;
    }
    if(!alloc_flag){
        printk("[ERRO]:user_task.c/1");
        while (True);
    }
    bitmap re_bitmap;
    //从0~3GB虚拟地址来映射
    re_bitmap.vaddr_header = vm_head;
    re_bitmap.target_addr_header = 0x00000000;
    //需要3GB空间映射
    re_bitmap.length = 0x00018000;  //需要96KB的存储空间存放
    bitmap_init_mem(re_bitmap);
    return re_bitmap;
}

//release all 96KB to kern vm
//need release 96/4 = 24 times;
void release_user_task_bitmap(bitmap bm){
    uint32_t val_4KB = 0x1000;
    uint32_t vm = bm.vaddr_header;
    for (int i = 0; i < 24; i++) {
        vmm_kern_release_one_page(vm);
        vm += val_4KB;
    }
}

static void pdt_mapping_helper(uint32_t pte_vaddr){
    //copy length is 1024B
    memcpy(pte_vaddr,get_pde(0xC0000000),1024);
}

static void pt_release_helper(uint32_t pdt_vaddr){
    //PAGE DIRECTORY DESC
}

//create pdt for user_task
//do two things: 1.alloc virtual memory from kern vm pool
//               2.mapping 3-4GB to kern page table (by using pointer)
uint32_t create_user_task_pdt(){
    uint32_t pdt_vaddr = vmm_kern_alloc();
    if(pdt_vaddr==KERN_VMM_ALLOC_ERRO){
        return USER_TASK_INIT_ERRO;
    }
    else{
        //clear mem
        memset((void *) pdt_vaddr, 0, 0x1000);
        //mapping......
        pdt_mapping_helper(pdt_vaddr);
    }
    return pdt_vaddr;
}

//release the mem when release user task
//do two things:  1.release the kern mem of page table
//                2.release the kern mem of page dir table(this is alloc when the task creates)
void release_user_task_pdt(uint32_t pdt_vaddr){
    pt_release_helper(pdt_vaddr);
    vmm_kern_release_one_page(pdt_vaddr);
}

//页表装载------------------------------
static void _active_pdt(uint32_t pdt_paddr){
    //内联汇编   修改cr3
    asm volatile("movl %0, %%cr3" : : "r" (pdt_paddr) : "memory");
}

static void _active_kern_pdt(){
    _active_pdt(kern_dir_table_paddr);
}

static void _active_user_pdt(uint32_t pdt_paddr){
    _active_pdt(pdt_paddr);
}
//页表装载------------------------------


void active_task(TCB_t* tcb_ptr){
    if(tcb_ptr->is_kern_thread){
        //内核线程同样需要重新装载页表      否者可能会访问到上一个用户进程的用户虚拟空间（0-3GB）
        _active_kern_pdt();
    }
    else{
        //激活用户进程页表
        uint32_t user_task_pdt_paddr = vmm_v2p(tcb_ptr->pdt_vaddr);
        if(user_task_pdt_paddr==V2P_ERROR){
            STOP(LOG_SRC_USER_TASK,"V2P Error!STOP!");
        }
        _active_user_pdt(user_task_pdt_paddr);
        //对于用户进程 需要更新tss的0特权级栈地址
        update_tss(tcb_ptr);
    }
}


//这个函数作为进程创建以后的线程执行的第一个函数   在这个函数中 会根据输入的目标function或者filename名字加载相应的用户进程，执行进程的初始化，最后执行伪中断退出到达3特权级
void start_user_task(start_user_task_params_t* params_ptr){
    if(params_ptr->is_from_file){
        STOP(LOG_SRC_USER_TASK,"File System is not exits!STOP!");
    }
    else{
        //检查function是否为NULL
        if(params_ptr->function == NULL){
            goto error_out;
        }
        else{
            return;
        }       
    }

    error_out:
        STOP(LOG_SRC_USER_TASK,"Main Function is NULL when create user task by function input!STOP!");
}

// 用于创建用户进程对应的线程   并且分配相应的用户进程资源
//args是start_user_task_params_t*类型的！！！
//注意：args中的内容不能被回收！！！也就是说调用create_user_task的函数不能退出，只能等待
void create_user_task(uint32_t tid,start_user_task_params_t*args){
	bool is_kern_thread =False;
	bitmap user_vmm_pool = create_user_task_bitmap();
	uint32_t pdt_vaddr = create_user_task_pdt();
    if (pdt_vaddr==USER_TASK_INIT_ERRO){
        STOP(LOG_SRC_USER_TASK,"Can`t Create New User Task Because Of Error When Alloc User Pdt!STOP!");
    }
	uint32_t page_counte = 1;
	uint32_t TCB_page = vmm_kern_alloc();
    if(TCB_page==KERN_VMM_ALLOC_ERRO){
        STOP(LOG_SRC_USER_TASK,"Can`t Create New User Task Because Of Error When Alloc TCB Page From Kernel VMM!STOP!");
    }
	create_thread(tid,start_user_task,(void*)args,TCB_page,page_counte,is_kern_thread,user_vmm_pool,pdt_vaddr);
}


void update_tss(TCB_t* tcb_ptr){

}

void change_esp(uint32_t stack_top_addr){

}

//用于清理中断栈 并且实现中断返回
//这个函数是创建user_task时跳转3特权级时调用 在interrupt中的清理中断栈实际上是在interrupt中实现的
//调用此函数之前需要把栈顶指针置于user_task中interrupt_stack顶
void handle_int_exit_stack(interrupt_stack_t * stack_top_ptr){

}



void user_task_test(){
    for(int i=0;i<1000;i++){
        printk("running!%d",i);
        bitmap bm = create_user_task_bitmap();
        release_user_task_bitmap(bm);
        //uint32_t addr =create_user_task_pdt();
        //release_user_task_pdt(addr);
    }
}