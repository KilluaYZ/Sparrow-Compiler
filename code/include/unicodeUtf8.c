#include "unicodeUtf8.h"
#include "common.h"

//返回value按照utf-8编码后的字节数
uint32_t getByteNumOfEncodeUtf8(int value){
    ASSERT(value > 0, "Can't encode negative value!");

    //单个ASCII字符需要1字节
    if(value <= 0x7f){
        return 1;
    }

    //此范围内的数值编码为UTF8需要2字节
    if(value <= 0x7ff){
        return 2;
    }

    //此范围内的数值编码为UTF8需要3字节
    if(value <= 0xffff){
        return 3;
    }

    //此范围内的数值编码为UTF8需要4字节
    if(value <= 0x10ffff){
        return 4;
    }

    //超出范围返回0
    return 0;
}

//把value编码为utf8后写入缓冲区buf，返回写入的字节数
uint8_t encodeUtf8(uint8_t* buf, int value){
    ASSERT(value > 0, "Can't encode negative value!");
    //按照大端写入缓冲区
    if(value <= 0x7f){
        *buf = value & 0x7f;
        return 1;
    }else if(value <= 0x7ff){
        //先写入高字节
        *buf++ = 0xc0 | ((value & 0x7c0) >> 6);
        //再写入低字节
        *buf = 0x80 | (value & 0x3f);
    }else if(value <= 0xffff){
        *buf++ = 0xe0 | ((value & 0xf000) >> 12);
        *buf++ = 0x80 | ((value & 0xfc0) >> 6);
        *buf = 0x80 | (value & 0x3f);
        return 3;
    }else if(value <= 0x10ffff){
        *buf++ = 0xf0 | ((value & 0x1c0000) >> 18);
        *buf++ = 0x80 | ((value & 0x3f000) >> 12);
        *buf++ = 0x80 | ((value & 0xfc0) >> 6);
        *buf = 0x80 | (value & 0x3f);
        return 4;
    }

    NOT_REACHED();
    return 0;
}

//返回解码UTF8的字节数
uint32_t getByteNumOfDecodeUtf8(uint8_t byte){
    //byte应该是UTF-8的最高1字节，如果指向了UTF8编码后面的低字节部分则返回0
    if((byte & 0xc0) == 0x80){
        return 0;
    }

    if((byte & 0xf8) == 0xf0){
        return 4;
    }

    if((byte & 0xf0) == 0xe0){
        return 3;
    }

    if((byte & 0xe0) == 0xc0){
        return 2;
    }

    //acii码
    return 1;
}

//解码以bytePtr为起始地址的UTF-8序列，其最大长度为length，若不是UTF-8则返回-1
int decodeUtf8(const uint8_t* bytePtr, uint32_t length){
    //若是1字节的ascii: 0xxxxxxx
    if(*bytePtr <= 0x7f){
        return *bytePtr;
    }

    int value;
    uint32_t remainintBytes;
    //先读取高1字节
    //根据高字节的高n位判断相应字节数的UTF-8编码
    if((*bytePtr & 0xe0) == 0xc0){
        //若是2字节的utf8
        value = *bytePtr & 0x1f;
        remainintBytes = 1;
    }else if((*bytePtr & 0xf0) == 0xe0){
        //若是3字节的utf8
        value = *bytePtr & 0x0f;
        remainintBytes = 2;
    }else if((*bytePtr & 0xf8) == 0xf0){
        //若是4字节的utf8
        value = *bytePtr & 0x07;
        remainintBytes = 3;
    }else {
        //非法编码
        return -1;
    }
    
    //如果UTF8被斩断了不再读过去了
    if(remainintBytes > length - 1){
        return -1;
    }

    //再读取低字节中的数据
    while(remainintBytes > 0){
        bytePtr++;
        remainintBytes--;
        if((*bytePtr & 0xc0) != 0x80){
            return -1;
        }

        //从次高字节往低字节，不断累加各字节的低6位
        value = value << 6 | (*bytePtr & 0x3f);
    }

    return value;
}