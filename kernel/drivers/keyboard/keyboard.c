//
// Created by root on 2020/11/9.
// device driver for keyboard
//
#include "types.h"
#include "keyboard.h"
#include "port.h"
#include "interrupt.h"
#include "printk.h"
#define KEYBOARD_INT_NO 33
#define KEYBOARD_DATA_PORT 0x60
void interrupt_handler_keyboard(uint32_t int_no,void * args){
    byte byte1 = inb(KEYBOARD_DATA_PORT);
    uint32_t val = ((uint32_t)(byte1))&0x000000FF;
    //default keyboard handler
    printk("keyboard!0x%h\n",val);
}

void keyboard_init(){
    register_interrupt(KEYBOARD_INT_NO, interrupt_handler_keyboard);
}
#undef KEYBOARD_INT_NO
#undef KEYBOARD_DATA_PORT