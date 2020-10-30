//
// Created by root on 2020/10/21.
//

#ifndef KERN_LOG_H
#define KERN_LOG_H

#define DEBUG_FLAGE    //设置DEBUG标志，当设置此标志时，DEBUG类会生效
#define WARNING warning_kern
#define INFO info_kern
#define ERROR error_kern
#define STOP stop_kern
#define DEBUG debug
typedef void * function(char * src , char * text);
void warning_kern(char* src,char* text);
void info_kern(char* src,char* text);
void error_kern(char* src,char* text);
void stop_kern(char* src,char* text);
void debug(function func,char *src,char*text);
#endif