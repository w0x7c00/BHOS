//映射了最后一页到页目录起始 在使用内核页表时可以直接通过最后4MB操作对应的内核页表
#include "vmm.h"
#include "pmm.h"
//内核已使用的页数量(1MB以下也是已使用的部分)
static int get_kern_used_page_count(){
    uint32_t size_bytes = kern_end-0;
    return (size_bytes+PAGE_SIZE-1)/PAGE_SIZE;     //向上取整
}

