/*定义虚拟机*/
#ifndef _VM_VM_H
#define _VM_VM_H

#include "common.h"
#include "header_obj.h"
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
