#include "vm.h"
#include "obj_fn.h"
#include "class.h"
#include<string.h>

//新建模块
ObjModule* newObjModule(VM* vm, const char* modName){
    ObjModule* objModule = ALLOCATE(vm,ObjModule);
    if(objModule == NULL){
        MEM_ERROR("allocate objModule failed!");
    }

    //ObjModule是信息对象，不属于任何一个类，所以class为NULL
    initObjHeader(vm,&objModuel->objHeader,OT_MODULE,NULL);

    StringBufferInit(&objModule->moduleVarName);
    ValueBufferInit(&objModuel->moduleVarValue);

    //核心模块名为NULL
    objModule->name = NULL;
    if(modName != NULL){
        objModule->name = newObjString(vm,modName,strlen(modName));
    }

    return objModule;
}

//创建类class的实例
ObjInstance* newObjInstance(VM* vm, Class* class){
    //参数class主要作用是提供类中field的数目
    ObjInstance* objInstance = ALLOCATE_EXTRA(vm,ObjInstance,sizeof(Value)*class->fieldNum);
    //在此关联对象的类为参数class
    initObjHeader(vm,&objInstance->objHeader, OT_INSTANCE,class);

    //初始化field为NULL
    uint32_t idx  = 0;
    while(idx < class->fieldNum){
        objInstance->fields[idx++] = VT_TO_VALUE(VT_NULL);
    }

    return objInstance;
}
