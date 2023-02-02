#include "vm.h"
#include <stdlib.h>
#include <utils.h>
#include "core.h"

//初始化虚拟机
void initVM(VM* vm){
    vm->allocatedBytes = 0;
    vm->curParser = NULL;
    vm->allObjects = NULL;
    StringBufferInit(&vm->allMethodNames);
    vm->allModules = newObjMap(vm);
    vm->curParser = NULL;
}

VM* newVM(){
    VM* vm = (VM*)malloc(sizeof(VM));
    if(vm == NULL){
        MEM_ERROR("allocate VM failed");
    }
    initVM(vm);
    buildCore(vm);
    return vm;
}