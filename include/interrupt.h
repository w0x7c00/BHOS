#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "types.h"
//中断服务例程函数指针类型   传入的是eax的值 指向函数参数压栈位置
typedef void* (*int_server_func_t)(uint32_t ,void *);

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


//这个结构使用来用户进程伪造中断返回切换权限级的
//供jmp_level_3使用
//包含（高地址到低地址）：CPU自动填充的结构（cs ss eflag eip esp error_code） 需要保存回填的结构（通用寄存器  特殊寄存器 中断号 其他段寄存器）
// 结构
//  high addr :       ss
//                               esp
//                                eflags
//                                cs
//                                eip
//                               错误号或0填充
// 								中断号
//                               eax
//                               ecx 
//                               edx
//                               ebx
//                               esp                    
//                               ebp
//                               esi
//                               edi
//                               es（扩展为32位）
//                               fs（扩展为32位）
//                               gs（扩展为32位）
//                               中断参数
// low addr            中断号

typedef
struct interrupt_stack{
	uint32_t int_no_1;//中断号
	void * args;   //中断参数
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t edi;  	
	uint32_t esi;  	
	uint32_t ebp;  	
	uint32_t esp_duplicate; //在pushad中重复压入的esp  需要重新命名
	uint32_t ebx;  	
	uint32_t edx;  	
	uint32_t ecx;  	
	uint32_t eax;
	uint32_t int_no_2;  //多余的中断 用于中断处理是时暂时存放  	
	uint32_t error_no;//中断错误号
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;
}__attribute__((packed)) interrupt_stack_t;

void cli();

void sti();

void idt_init();

bool cli_condition();

void sti_condition(bool condition);

void register_interrupt(int int_no, int_server_func_t target_func);

#endif