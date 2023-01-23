// 定义基本的数据结构
#ifndef _INCLUDE_COMMON_H
#define _INCLUDE_COMMON_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//定义了VM,Parser,Class。完整实现分别在其他的文件中
typedef struct vm VM;
typedef struct parser Parser;
typedef struct class Class;

#define bool char
#define true 1
#define false 0
//函数中一部分形参没有被使用，定义UNUSED通过gcc内部属性告诉gcc不要报警
#define UNUSED __attribute__ ((unused))

//DEBUG打印
#ifndef DEBUG
    #define ASSERT(condition, errMsg) \
        do { \
            if(!(condition)) {\
                fprintf(stderr,"ASSERT failed! %s:%d In function %s(): %s\n",\
                __FILE__, __LINE__, __func__, errMsg);\
                abort();\
            }\
        }while(0);
    
#else
    #define ASSERT(condition,errMsg) ((void)0)
#endif

//不可能到达，有些程序分支是不可能被执行到的，如果被执行到，说明程序出错，
//为了检测这种情况，定义该宏，将其安插到不可能到达的分支中
#define NOT_REACHED()\
    do {\
        fprintf(stderr,"NOT_REACHED: %s:%d In function %s()\n",\
        __FILE__, __LINE__, __func__);\
        while(1);\
    }while(0);
#endif