#include "kern_log.h"
#include "printk.h"
#include "vga_basic.h"
void info_kern(char* src,char* text){
    printk("[INFO][%s]%s\n",src,text);
}

void error_kern(char* src,char* text){
    printk_color("[ERROR][%s]%s\n",black,red,src,text);
}

void warning_kern(char* src,char* text){
    printk_color("[WARNING][%s]%s\n",black,yellow,src,text);
}

void stop_kern(char* src,char* text){
    //printk("[STOP][%s]%s\n",src,text);
    printk_color("[STOP][%s]%s\n",white,black,src,text);
    while(1){}
}

void debug(function func,char *src,char*text){
    #ifdef DEBUG_FLAGE
    func(src,text);
    #endif
}