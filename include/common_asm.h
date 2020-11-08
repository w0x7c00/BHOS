#ifndef COMMON_ASM_H
#define COMMON_ASM_H

#include "types.h"

void reload_gdt();
void get_eflag(uint32_t * eflag);
void exit_int(void * esp);
void reload_kern_page_table(uint32_t pdt_paddr);
#endif