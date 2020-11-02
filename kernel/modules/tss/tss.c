/*
/kernel/module/tss/tss.c
by 不吃香菜的大头怪
2020-10-30 create
2020-10-30 last modify
Description:  Funtion For Init CPU TSS And Update TSS`s ESP0 When Change Task For Running
*/

#include "types.h"
#include "tss.h"
#include "string.h"
#include "printk.h"
#include "common_asm.h"
#include "threads.h"
#include "kern_log.h"
#define GDT_DESC_SIZE 8     //一个描述符8个字节
#define TSS_ATTR_LOW   0x89//10001001//(1<<7)+(0<<5)+(0<<4)+9
#define TSS_ATTR_HIGH  0x80//10000000//(1<<7)+(0<<6)+(0<<5)+(0<<4)+0x0
#define LOG_SRC_TSS "TSS"
tss_t tss_cpu0;      //存放cpu0的tss    全局变量 不会被栈回收

static  tss_desc_t* get_tss_desc(){
    //GDT_BASE的地址是虚拟地址
    uint32_t gdt_head_vaddr = (uint32_t)&GDT_BASE;
    uint32_t tss_desc_vaddr = gdt_head_vaddr+GDT_DESC_SIZE*TSS_CPU0_DESC_POS_ON_GDT;
    return (tss_desc_t*)tss_desc_vaddr;
}

static tss_desc_t create_tss_desc(uint32_t desc_base,uint32_t limit,uint8_t attr_low,uint8_t attr_high){
    tss_desc_t desc;
    desc.limit_low_word = (uint16_t)(limit & 0x0000FFFF);
    desc.base_low_word = (uint16_t)(desc_base&0x0000FFFF);
    desc.base_mid_byte = (uint8_t)((desc_base&0x00FF0000)>>16);
    desc.attr_low_byte = (uint8_t)attr_low;
    desc.limit_high_attr_high = ((limit&0x000F0000)>>16)+(uint8_t)(attr_high);
    desc.base_high_byte = (uint8_t)(desc_base>>24);
    return desc;
}

//这个函数用来验证TSS是否加载成功
//加载成功后TSS描述符的B位会被置为1
static uint32_t get_TSS_desc_high_word(){
    tss_desc_t * tss_d = get_tss_desc();
    uint32_t * p = (uint32_t*)tss_d;
    return *(p+1);
}

void tss_init(){
    uint32_t tss_size = sizeof(tss_t);
    tss_desc_t* tss_cpu0_desc_ptr = get_tss_desc();
    bzero(&tss_cpu0,tss_size);
    tss_cpu0.io_base =tss_size;
    tss_cpu0.ss0 = SELECTOR_DATA_MEM&0x0000FFFF;    //内核数据段
    *tss_cpu0_desc_ptr=create_tss_desc(&tss_cpu0,tss_size-1,TSS_ATTR_LOW,TSS_ATTR_HIGH);   
    //重载GDT
    uint32_t befor_load_val = get_TSS_desc_high_word();
    reload_gdt();
    //加载TSS
    asm volatile("ltr %w0" : : "r" (SELECTOR_TSS_CPU0_MEM));
    uint32_t after_load_val = get_TSS_desc_high_word();
    //装载tss后 cpu会修改tss描述符B位 所以高32位会变化
    if(after_load_val!=befor_load_val){
        INFO(LOG_SRC_TSS,"Init TSS Success!");
    }
    else{
        STOP(LOG_SRC_TSS,"Fail To Load TSS!STOP!");
    }
}

void tss_update(TCB_t* tcb_ptr){
    tss_cpu0.esp0 = (uint32_t*)((uint32_t)tcb_ptr+tcb_ptr->page_counte*PAGE_SIZE);
}

void tss_test(){
    printk("GDT_BASE:%h\nGDT_PTR:0x%h\n",&GDT_BASE,&gdt_ptr);
}