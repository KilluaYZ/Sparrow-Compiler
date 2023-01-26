#ifndef _OBJECT_RANGE_H
#define _OBJECT_RANGE_H

#include "class.h"

// range对象定义
typedef struct
{
    ObjHeader objHeader;
    int from; // 范围起始
    int to;   // 范围结束

} ObjRange;

ObjRange *newObjRange(VM *vm, int from, int to);

#endif