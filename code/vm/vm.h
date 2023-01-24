/*定义虚拟机*/
#ifndef _VM_VM_H
#define _VM_VM_H

#include "common.h"

struct vm{
    //累计已分配的内存量
    uint32_t allocatedBytes;
    //当前词法分析器
    Parser* curParser;
};

void initVM(VM* vm);
VM* newVM();


#endif
