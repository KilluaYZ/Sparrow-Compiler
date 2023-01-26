#include "obj_map.h"
#include "class.h"
#include "obj_range.h"
#include "obj_string.h"
#include "vm.h"

//创建新的map对象
ObjMap *newObjMap(VM *vm){
    ObjMap* objMap = ALLOCATE(vm, ObjMap);
    initObjHeader(vm, &objMap->objHeader, OT_MAP, vm->mapClass);
    objMap->capacity = objMap->count = 0;
    objMap->entries = NULL;
    return objMap;
}

//计算数字的哈希值
static uint32_t hashNum(double num){
    Bits64 bits64;
    bits64.num = num;
    return bits64.bits32[0] ^ bits64.bits32[1];
}

//计算对象的hash值
static uint32_t hashObj(ObjHeader* objHeader){
    switch(objHeader->type){
        case OT_CLASS:  //计算class哈希
            return hashString(((Class*)objHeader)->name->value.start,((Class*)objHeader)->name->value.length);
        case OT_RANGE:
            ObjRange* objRange = (ObjRange*)objHeader;
            return hashNum(objRange->from) & hashNum(objRange->to);
        case OT_STRING:
            return ((ObjString*)objHeader)->hashCode;
        default:
            RUN_ERROR("the object is not hashable. the hashable are string, range and class.");
    }
    return 0;
}

//根据value的类型调用相应的哈希函数
static uint32_t hashValue(Value value){
    switch(value.type){
        case VT_FALSE:
            return 0;
        case VT_NULL:
            return 1;
        case VT_NUM:
            return hashNum(value.num);
        case VT_TRUE:
            return 2;
        case VT_OBJ:
            return hashObj(value.objHeader);
        default:
            RUN_ERROR("unsupport type hashed!");
    }
    return 0;
}

static bool addEntry(Entry* entries, uint32_t capacity, Value key, Value value){
    uint32_t index = hashValue(key) % capacity;
    //通过开放地址法取寻找可用的slot
    while(true){
        //找到空闲的slot，说明目前没有此key，直接赋值返回
        if(entries[index].key.type == VT_UNDEFINED){
            entries[index].key = key;
            entries[index].value = value;
            //添加了新的key，返回true
            return true;
        }else if(valueIsEqual(entries[index].key, key)){
            //key已经存在，仅仅更新就行
            entries[index].value = value;
            //只是修改了旧的key，返回false
            return false;
        }
        //开放地址法，尝试下一个slot
        index = (index + 1) % capacity;
    }
}

//使对象objMap的容量调整到newCapacity
static void resizeMap(VM* vm, ObjMap* objMap, uint32_t newCapacity){
    //先建立一个新的entry
    Entry* newEntries = ALLOCATE_ARRAY(vm, Entry, newCapacity);
    uint32_t idx = 0;
    while(idx < newCapacity){
        newEntries[idx].key = VT_TO_VALUE(VT_UNDEFINED);
        newEntries[idx].value = VT_TO_VALUE(VT_FALSE);
        idx++;
    }

    //再遍历老数组，将有值的部分插入到新数组
    if(objMap->capacity > 0){
        Entry* entryArr = objMap->entries;
        idx = 0;
        while(idx < objMap->capacity){
            if(entryArr[idx].key.type != VT_UNDEFINED){
                addEntry(newEntries, newCapacity, entryArr[idx].key, entryArr[idx].value);
            }
            idx++;
        }
    }

    //将老entry数组空间回收
    DEALLOCATE_ARRAY(vm, objMap->entries, objMap->count);
    //更新指针指向新的entry数组
    objMap->entries = newEntries;
    //更新容量
    objMap->capacity = newCapacity;
}

//objMap中查找key对应的entry
static Entry* findEntry(ObjMap* objMap, Value key){
    if(objMap->capacity == 0){
        return NULL;
    }

    //使用开放地址法探测
    uint32_t index = hashValue(key) % objMap->capacity;
    Entry* entry;
    while(true){
        entry = &objMap->entries[idx];

        //若该slot中的entry正好是该key的entry，找到返回
        if(valueIsEqual(entry->key,key)){
            return entry;
        }

        //key为VT_UNDEFINED，value为VT_TRUE，表示探测链未断，可继续探测
        //key为VT_UNDEFINED，value为VT_FALSE，表示探测链已断，不可继续探测
        if(VALUE_IS_UNDEFINED(entry->key) && VALUE_IS_FALSE(entry->value)){
            return NULL;
        }

        //继续向下探测
        index = (index + 1) % objMap->capacity;
    }
}

//在objMap中添加新的entry
void mapSet(VM *vm, ObjMap *objMap, Value key, Value value){
    //当容量利用率达到80%时扩容
    if(objMap->count + 1 > objMap->capacity * MAP_LOAD_PERCENT){
        uint32_t newCapacity = objMap->capacity * CAPACITY_GROW_FACTOR;
        if(newCapacity < MIN_CAPACITY){
            newCapacity = MIN_CAPACITY;
        }
        resizeMap(vm,objMap,newCapacity);
    }

    //若创建了新的key则使objMap->count加1
    if(addEntry(objMap->entries, objMap->capacity,key, value)){
        objMap->count++;
    }
}

//获取objMap中key对应的value
Value mapGet(ObjMap *objMap, Value key){
    Entry* entry = findEntry(objMap,key);
    if(entry == NULL){
        return VT_TO_VALUE(VT_UNDEFINED);
    }
    return entry->value;
}

//回收objMap.entries占用的空间
void clearMap(VM *vm, ObjMap *objMap){
    DEALLOCATE_ARRAY(vm, objMap->entries, objMap->count);
    objMap->entries = NULL;
    objMap->capacity = objMap->count = 0;
}

//删除objMap中的key，返回map[key]
Value removeKey(VM *vm, ObjMap *objMap, Value key){
    Entry* entry = findEntry(objMap, key);
    if(entry == NULL){
        return VT_TO_VALUE(VT_NULL);
    }

    //设置开放地址的伪删除
    Value value = entry->value;
    entry->key = VT_TO_VALUE(VT_UNDEFINED);
    entry->value = VT_TO_VALUE(VT_TRUE);

    objMap->count--;
    if(objMap->count == 0){
        //若删除该entry后map为空就回收该空间
        clearMap(vm, objMap);
    }else if(objMap->count < objMap->capacity / (CAPACITY_GROW_FACTOR) * MAP_LOAD_PERCENT &&objMap->count > MIN_CAPACITY){
        //如果map利用率太低，就缩小map空间
        uint32_t newCapacity = objMap->capacity / CAPACITY_GROW_FACTOR;
        if(newCapacity < MIN_CAPACITY){
            newCapacity = MIN_CAPACITY;
        }
        resizeMap(vm,objMap,newCapacity);
    }
    return value;
}