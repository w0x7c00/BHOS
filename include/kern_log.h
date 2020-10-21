//
// Created by root on 2020/10/21.
//

#ifndef KERN_LOG_H
#define KERN_LOG_H


#define WARNING warning_kern
#define INFO info_kern
#define ERROR error_kern

void warning_kern(char* src,char* text);
void info_kern(char* src,char* text);
void error_kern(char* src,char* text);

#endif