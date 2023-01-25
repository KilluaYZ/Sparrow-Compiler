#ifndef _OBJECT_OBJ_STRING_H
#define _OBJECT_OBJ_STRING_H

#include "header_obj.h"

//字符串对象的定义
typedef struct {
    ObjHeader objHeader;
    uint32_t hashCode;  //字符串hash值
    CharValue value;
} ObjString;

uint32_t hashString(char* str, uint32_t length);
void hashObjString(ObjString* objString);
ObjString* newObjString(VM* vm, const char* str, uint32_t length);


#endif