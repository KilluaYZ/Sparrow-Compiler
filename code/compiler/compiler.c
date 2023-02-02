#include <string.h>
#include "compiler.h"
#include "parser.h"
#include "core.h"

#if DEBUG
#include "debug.h"
#endif

//编译单元
struct compileUnit{
	//所编译的函数
	ObjFn* fn;

	//作用域中允许的局部变量的个数上限
	LocalVar localVars[MAX_LOCAL_VAR_NUM];

	//已分配的局部变量个数
	uint32_t localVarNum;

	//记录本层函数所引用的Upvalue
	Upvalue upvalues[Max_UPVALUE_NUM];

	//当前正在编译的代码所处的作用域
	int scopeDepth;

	//当前正在编译的循环层
	Loop * curLoop;

	//当前正编译的类的编译信息
	ClassBookKeep* enclosingClassBK;

	//包含此编译单元的编译单元，即直接外层
	struct compileUnit* enclosingUnit;

	//当前parser
	Parser* curParser;

};

//在模块objModule中定义名为Name，值为value的模块变量
int defineModuleVar(VM* vm, ObjModule* objModule, const char* name, uint32_t length, Value value){
	if(length > MAX_ID_LEN){
		//也许name指向的字符串不以\0结尾，因此需要将其从源码串中拷贝出来
		char id[MAX_ID_LEN] = {'\0'};
		memcpy(id, name, length);
		//本函数可能在编译源码文件之前就调用了，那时候还没有创建parser，因此报错要分情况
		if(vm->curParser != NULL){
			//编译源码文件
			COMPILE_ERROR(vm->curParser, "length of identifier \"%s\" should be no more than %d", id, MAX_ID_LEN);
		}else{
			//编译源码前调用，比如加载核心模块时会调用本函数
			MEM_ERROR("length of identifier \"%s\" should be no more than %d",id, MAX_ID_LEN);
		}
	}

	//从模块变量名中查找变量，若不存在就添加
	int symbolIndex = getIndexFromSymbolTable(&objModule->moduleVarName,name,length);
	if(symbolIndex == -1){
		//添加变量名
		symbolIndex = addSymbol(vm, &objModule->moduleVarName, name, length);
		//添加变量值
		ValueBufferAdd(vm, objModule->moduleVarValue,value);
	}else if(VALUE_IS_NUM(objModule->moduleVarValue.datas[symbolIndex])){
		//若遇到之前预先声明的模块变量的定义，在此为其赋予正确的值
		objModule->moduleVarValue.datas[symbolIndex] = value;
	}else{
		//已定义返回-1，防止重定义
		symbolIndex = -1;
	}

	return symbolIndex;
}
