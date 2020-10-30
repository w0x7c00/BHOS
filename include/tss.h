#ifndef TSS_H
#define TSS_H
#include "types.h"
#include "threads.h"
//CPU0的tss在gdt的第6项
#define TSS_CPU0_DESC_POS_ON_GDT 6
extern uint32_t gdt_ptr;
extern uint32_t GDT_BASE;
extern uint16_t SELECTOR_TSS_CPU0_MEM;
extern uint16_t SELECTOR_DATA_MEM;
typedef struct tss_desc{
    uint16_t limit_low_word;
    uint16_t base_low_word;
    uint8_t base_mid_byte;
    uint8_t attr_low_byte;
    uint8_t limit_high_attr_high;
    uint8_t base_high_byte;
}__attribute__((packed)) tss_desc_t;

//tss结构
typedef struct tss
{
    uint32_t backlink;
    uint32_t * esp0;
    uint32_t  ss0;
    uint32_t *esp1;
    uint32_t ss1;
    uint32_t *esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t (*eip)(void);
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint32_t trace;
    uint32_t io_base;
}__attribute__((packed)) tss_t;


void tss_test();
void reload_gdt();
void tss_init();
void tss_update(TCB_t* tcb_ptr);

#endif