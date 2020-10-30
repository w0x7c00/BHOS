#ifndef USER_TASK_H
#define USER_TASK_H
#include "types.h"


typedef struct start_user_task_params
{
    bool  is_from_file;    //判断此进程的执行函数是否需要重文件系统中加载到内存中
    void * function;         //如果没有设置的话为NULL    用于执行对应的函数
    void * args;     //执行参数
    uint32_t fd;     //如果需要加载 获取加载目标文件  文件描述符
} start_user_task_params_t;


typedef int USER_TASK_STATUS;
#define USER_TASK_INIT_ERRO 0xFFFFFFFF
void user_task_test();
#endif