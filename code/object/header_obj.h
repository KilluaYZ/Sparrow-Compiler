#ifndef _OBJECT_HEADER_OBJ_H
#define _OBJECT_HEADER_OBJ_H

#include"utils.h"
//对象类型
typedef enum{
    OT_CLASS,       //此项是class类型，以下都是object类型
    OT_LIST,        //列表
    OT_MAP,         //散列数组
    OT_MODULE,      //模块作用域
    OT_RANGE,       //range类型，步进为1的数字范围
    OT_STRING,      //字符串
    OT_UPVALUE,     //闭包中提到的自由变量
    OT_FUNCTION,    //函数
    OT_CLOSURE,     //闭包
    OT_INSTANCE,    //实例
    OT_THREAD       //线程
}ObjType;

//对象头，用于记录元信息和垃圾回收，在面向对象的脚本语言中是万物的基础
typedef struct objHeader{
    ObjType type;   //对象类型
    bool isDark;    //对象是否可达，在垃圾回收中用
    Class* class;   //对象所属的类
    struct objHeader* next; //指向下一个创建的对象，链接所有已分配对象

} ObjHeader;

//value类型
typedef enum{
    VT_UNDIFINED,   //未定义
    VT_NULL,        //空值，不代表未定义
    VT_FALSE,       //布尔假
    VT_TRUE,        //布尔真
    VT_NUM,         //数字类型
    VT_OBJ          //值为对象，指向对象头
}ValueType;

//通用的值结构
typedef struct {
    ValueType type;
    union{
        double num;
        ObjHeader* objHeader;
    };
}Value;

DECLARE_BUFFER_TYPE(Value)

void initObjHeader(VM* vm,ObjHeader* objHeader, ObjType objType, Class* class);

#endif