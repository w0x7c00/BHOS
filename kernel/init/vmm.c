//一般来讲 内核大小小于4MB 所以初始映射的4MB够用了
//映射了最后一页到页目录起始 在使用内核页表时可以直接通过最后4MB操作对应的内核页表
#include "vmm.h"
#include "pmm.h"
#include "bitmap.h"
#include "printk.h"
#include "types.h"
//内核已使用的页数量(1MB以下也是已使用的部分)

bitmap kern_vmm_pool;


static int get_kern_used_page_count(){
    uint32_t size_bytes = kern_end-0;
    return (size_bytes+PAGE_SIZE-1)/PAGE_SIZE;     //向上取整
}

//初始化内核虚拟空间
//预分配已使用内核空间
static void kern_vmm_init(){    
    kern_vmm_pool.target_addr_header = 0xC0000000;   //内核空间起始地址
    kern_vmm_pool.length = 0x8000;     //总共1GB空间
    kern_vmm_pool.vaddr_header = kern_bitmap;
    bitmap_init_mem(kern_vmm_pool);
    int page_used = get_kern_used_page_count();
    if(page_used>1024){
        printk("\nERRO!don`t have enough vm page for kernel when init!\n");
        while (True){
            //内核暂停
        }
    }
    for(int i = 0; i<page_used ;i++){
        bitmap_alloc(kern_vmm_pool); //将内核空间在内存池中置为繁忙   此处可做性能优化
    }    
    //将内核虚拟内存管理页（最后1024页）在内存池中置为繁忙
    //1024页占用128字节 所以要将bitmap最后128字节清零
    int temp_cnt =0;
    for( uint32_t probe = kern_bitmap+0x8000-1;temp_cnt<128;temp_cnt++,probe--){
        *((byte*)probe) = 0;
    }
}

void vmm_init(){
       kern_vmm_init(); 
}

static uint32_t vmm_get_page_vaddr_by_target(uint32_t target){
    target = target&0xFFFFF000;
    uint32_t addr_high10 = 0x3FF<<22;     //高10位
    uint32_t addr_middle10 = (target>>10) & 0x003FF000;//中间10位为target高10位
    uint32_t addr_low12 = ((target>>12)&0x3FF)*4;//低12位为target中间10位×4
    return addr_high10|addr_middle10|addr_low12;
}

//修改页表时的地址结构：
//      1111111111_xxxxxxxxxx_xxxxxxxxxx
//前十位为1表示页目录表最后一项 指向页目录表起始地址
// 中间十位表示在1024项页目录表中的索引
//最后12位表示在页表中的索引（每张页表1024项 共4096B 也就是12位可表达的最大范围）
uint32_t vmm_kern_alloc_one_page(uint32_t target){
        
}

uint32_t vmm_kern_alloc(){

}

void vmm_kern_release_one_page(uint32_t target){

}

static void user_vmm_init(){

}

void vmm_test(){

}