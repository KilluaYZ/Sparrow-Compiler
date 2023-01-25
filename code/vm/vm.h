/*定义虚拟机*/
#ifndef _VM_VM_H
#define _VM_VM_H

#include "common.h"

struct vm{
    Class* stringClass;
    Class* fnClass;
    //所有已分配对象链表
    ObjHeader* allObjects;
    //累计已分配的内存量
    uint32_t allocatedBytes;
    //当前词法分析器
    Parser* curParser;
};

void initVM(VM* vm);
VM* newVM();


#endif
