#ifndef _OBJECT_META_OBJ_H
#define _OBJECT_META_OBJ_H
/*定义对象的一些信息*/
#include"obj_string.h"


//模块对象
/*模块就是在某独立作用域中定义的代码的集合，但是他不储存代码，代码保存在代码单元中*/
typedef struct 
{
    ObjHeader objHeader;
    SymbolTable moduleVarName;      //模块中的模块变量名
    ValueBuffer moduleVarValue;     //模块中的模块变量值
    ObjString* name;                //模块名
}ObjModule;

//对象实例
/*是通用的对象结构，与class是储存类的方法的结构不同，它是储存对象的域的结构*/
typedef struct

{
    //用于查找它所属的类
    ObjHeader objHeader;
    //具体的字段
    Value fields[0];
}ObjInstance;

ObjModule* newObjModule(VM* vm, const char* modName);
ObjInstance* newObjInstance(VM* vm,Class* class);

#endif
