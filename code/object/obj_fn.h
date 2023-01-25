#ifndef _OBJECT_OBJ_FN_H
#define _OBJECT_OBJ_FN_H

#include "meta_obj.h"
#include "uitls.h"

//在函数中的调试结构
typedef struct {
    char*       fnName;         //函数名
    IntBuffer   lineNo;         //行号
}FnDebug;

//函数对象
typedef struct{
    ObjHeader   objHeader;      
    ByteBuffer  instrStream;        //函数编译后的指令流
    ValueBuffer constants;          //函数中的常量表
    ObjModule*  module;             //本函数所属的模块
    uint32_t    maxStackSlotUsedNum;//本函数最多需要的栈空间，是栈使用空间的峰值
    uint32_t    upvalueNum;         //本函数所涵盖的upvalue数量
    uint8_t     argNum;             //函数期望的参数个数
#if DEBUG
    FnDebug*    debug;
#endif
}ObjFn;

//upvalue对象
typedef struct upvalue{
    ObjHeader objHeader;
    Value* localVarPtr;             //栈是个Value类型的数组，localVarPtr指向upvalue所关联的局部变量
    Value closedUpvalue;            //已经被关闭的upvalue
    struct upvalue* next;           //用以链接openUpvalue链表
}ObjUpvalue;


//闭包对象
typedef struct{
    ObjHeader objHeader;
    ObjFn* fn;                      //闭包中所要引用的函数
    ObjUpvalue* upvalues[0];        //储存函数的closed upvalues
}ObjClosure;

//函数的调用框架，即栈帧
typedef struct{
    uint8_t*    ip;                 //程序计数器，指向下一个将被执行的指令
    ObjClosure* closure;            //在本frame中执行的闭包函数
    
    //frame是共享therad.stack
    //此项用于指向本frame所在thread运行时栈的起始地址
    Value* stackStart;
}Frame;

#define INITIAL_FRAME_NUM 4
ObjUpvalue* newObjUpvalue(VM* vm, Value* localVarPtr);
ObjClosure* newObjClosure(VM* vm,ObjFn* objFn);
ObjFn* newObjFn(VM* vm, ObjModule* objModule, uint32_t maxStackSlotUsedNum);

#endif