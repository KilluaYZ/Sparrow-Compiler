#ifndef _OBJECT_OBJ_THREAD_H
#define _OBJECT_OBJ_THREAD_H

#include "obj_fn.h"

//线程对象
typedef struct objThread{
    ObjHeader objHeader;
    Value* stack;           //运行时栈的栈底，相当于是一个大栈
    Value* esp;             //运行时栈的栈顶
    uint32_t stackCapacity; //栈容量
    Frame* frames;          //调用框架，即函数运行时栈数组，发生一个调用时，就从此数组中分配一个运行时栈，相当于是若干个小栈
    uint32_t usedFrameNum;  //已使用的frame数量
    uint32_t frameCapacity; //frame容量

    //打开的upvalue的链表首节点
    ObjUpvalue* openUpvalues;
    //当前thread的调用者，作用是本线程退出，控制权将回到调用者
    struct objThread* caller;

    //导致运行时错误的对象会放在此处，否则为空
    Value errorObj;
} ObjThread;

ObjThread* newObjThread(VM* vm, ObjClosure* objClosure);

#endif