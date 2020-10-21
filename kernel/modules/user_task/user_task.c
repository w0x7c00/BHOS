//2020 9 24
//by buchixiangcaidedatouguai
#include "user_task.h"
#include "bitmap.h"
#include "vmm.h"
#include "printk.h"
#include "string.h"

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

void user_task_test(){
    for(int i=0;i<1000;i++){
        printk("running!%d",i);
        bitmap bm = create_user_task_bitmap();
        release_user_task_bitmap(bm);
        //uint32_t addr =create_user_task_pdt();
        //release_user_task_pdt(addr);
    }
}