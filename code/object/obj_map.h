#ifndef _OBJECT_OBJ_MAP_H
#define _OBJECT_OBJ_MAP_H

#include"header_obj.h"
#define MAP_LOAD_PERCENT 0.8

//key->value 对儿
typedef struct{
    Value key;
    Value value;
} Entry;

//map对象定义
typedef struct
{
    ObjHeader objHeader;
    uint32_t capacity;      //Entry的数量，包括已使用和未使用的
    uint32_t count;         //已使用的entry的数量
    Entry* entries;         //Entry数组
} ObjMap;

ObjMap* newObjMap(VM* vm);
void mapSet(VM* vm, ObjMap* objMap, Value key ,Value value);
Value mapGet(ObjMap* objMap, Value key);
void clearMap(VM* vm, ObjMap* objMap);
Value removeKey(VM* vm, ObjMap* objMap, Value key);


#endif