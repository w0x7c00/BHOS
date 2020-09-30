#ifndef BITMAP_H
#define BITMAP_H

#include "types.h"
#define PAGE_SIZE 4096
#define BITMAP_RETURN_ERRO 0xffffffff        
#define BITMAP_BUSY 0
#define BITMAP_AVL 1
typedef 
struct bitmap{
    uint32_t vaddr_header;      /*bitmap存放处起始虚拟地址*/
    uint32_t length ;      /*bitmap存放空间长度(字节)*/
    uint32_t target_addr_header ;     /*管理目标区域的起始地址*/
} bitmap;


void bitmap_init_mem(bitmap bm);
uint32_t bitmap_alloc_one_page(bitmap bm,uint32_t target);
void bitmap_release_one_page(bitmap bm,uint32_t target);
uint32_t bitmap_alloc(bitmap bm);

void bitmap_test();

#endif