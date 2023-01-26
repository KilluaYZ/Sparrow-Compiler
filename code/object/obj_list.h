#ifndef _OBJECT_OBJ_LIST_H
#define _OBJECT_OBJ_LIST_H

#include "class.h"
#include "vm.h" 

//list对象定义
typedef struct{
    ObjHeader objHeader;
    //list中的对象
    ValueBuffer elements;
} ObjList;

ObjList* newObjList(VM* vm,uint32_t elementNum);
Value removeElement(VM* vm,ObjList* objList,uint32_t index);
void insertElement(VM* vm,ObjList* objList,uint32_t index, Value value);

#ifndef