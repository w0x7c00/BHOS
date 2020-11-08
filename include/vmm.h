#ifndef VMM_H
#define VMM_H

#include "types.h"
#include "threads.h"
#define PAGE_SIZE 4096
#define PAGE_DESC_P  0x1//1b
#define PAGE_DESC_RW_R  0x0//00b
#define PAGE_DESC_RW_W  0x2//10b
#define PAGE_DESC_US_U  0x4//100b
#define PAGE_DESC_US_S  0x0//000b
#define PAGE_DESC_G 0x100//100000000b//全局位 会存放于TLB缓存中的页
#define KERN_VMM_ALLOC_ERRO 0xFFFFFFFF
#define USER_VMM_ALLOC_ERRO 0xFFFFFFFF
#define V2P_ERROR 0xFFFFFFFF

// 获取链接器变量 
//内核加载起始位置-结束位置（物理内存）
extern uint8_t kern_start[];
extern uint8_t kern_end[];
extern uint32_t kern_bitmap;    //内核虚拟空间管理位图虚拟地址
extern uint32_t kern_dir_table_paddr;
extern uint32_t kern_page_table_paddr;
void reload_kern_page();
void vmm_init();
void vmm_pre_init();
void vmm_test();
uint32_t vmm_kern_alloc();
uint32_t vmm_kern_alloc_one_page(uint32_t target);
void vmm_kern_release_one_page(uint32_t target);

uint32_t get_pde(uint32_t target);
uint32_t get_pte(uint32_t target);
uint32_t vmm_v2p(uint32_t vaddr);
uint32_t vmm_user_alloc_one_page(TCB_t * tcb_ptr,uint32_t vaddr);

#endif