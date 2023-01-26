/*定义虚拟机*/
#ifndef _VM_VM_H
#define _VM_VM_H

#include "common.h"
#include "header_obj.h"
#include "obj_map.h"
#include "obj_thread.h"

//虚拟机执行结果
//如果执行无误，可以将字符码输出到文件缓存，避免下次重新编译
typedef enum vmResult{
    VM_RESULT_SUCCESS,
    VM_RESULT_ERROR
} VMResult;

struct vm{
    Class* stringClass;
    Class* fnClass;
    Class* classOfClass;
    Class* objectClass;
    Class* mapClass;
    Class* rangeClass;
    Class* listClass;
    Class* nullClass;
    Class* boolClass;
    Class* numClass;
    Class* threadClass;
    //所有已分配对象链表
    ObjHeader* allObjects;
    //累计已分配的内存量
    uint32_t allocatedBytes;
    //当前词法分析器
    Parser* curParser;
    ObjMap* allModules;
    ObjThread* curThread;
    SymbolTable allMethodNames;
};

void initVM(VM* vm);
VM* newVM();


#endif
