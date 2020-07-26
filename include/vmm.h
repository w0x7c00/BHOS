#ifndef     VMM_H
#define    VMM_H

#include "types.h"
#define PAGE_SIZE 4096
#define PAGE_DESC_P  1
#define PAGE_DESC_RW_R 00b
#define PAGE_DESC_RW_W 10b
#define PAGE_DESC_US_U  100b
#define PAGE_DESC_US_S  000b
#define PAGE_DESC_G 100000000b//全局位 会存放于TLB缓存中的页

// 获取链接器变量 
//内核加载起始位置-结束位置（物理内存）
extern uint32_t kern_start;
extern uint32_t kern_end;
extern uint32_t kern_dir_table;   //内核页目录物理地址
extern uint32_t kern_page_table;     //内核页表起始物理地址
extern uint32_t kern_bitmap;    //内核虚拟空间管理位图虚拟地址

void vmm_init();
#endif