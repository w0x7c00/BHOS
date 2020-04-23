#ifndef PRINTK_H
#define PRINTK_H
#include "types.h"
#include "vga_basic.h"
void printkDebug();

void printkbasic(char *format_str,char *m); //基础打印函数 

void printk(char *input_str,...);

void printk_color(char *format_str,vga_color_t back, vga_color_t fore,...);

void insert_str(char *inserted_str,char *inserting_str,uint32_t offset);

#endif