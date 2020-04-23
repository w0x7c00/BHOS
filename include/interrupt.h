#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "types.h"
//中断服务例程函数指针类型   传入的是eax的值 指向函数参数压栈位置
typedef void* (*int_server_func_t)(void *);    

typedef
struct int_disc{
	uint16_t offset_low;
	uint16_t selector;
	uint8_t default_bit8;     //未使用5位+默认为0的3位 
	uint8_t inf;          //TYPE S DPL P 描述字段（共八位）
	uint16_t offset_high;
}__attribute__((packed)) interrupt_discripter_t;

typedef
struct lidt_target_t {
    uint16_t limit;     // 限长
    uint32_t base;      // 基址
} __attribute__((packed)) lidt_target_t;

void idt_init();
#endif