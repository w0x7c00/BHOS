#include "printk.h"
#include "types.h"

void print_debug_1(uint32_t val){
    printk("debug1:0x%h\n",val);
}