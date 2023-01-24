/*UTF-8编码和解码*/
#ifndef _INCLUDE_UNICODEUTF8_H
#define _INCLUDE_UNICODEUTF8_H

#include<stdint.h>
uint32_t getByteNumOfEncodeUtf8(int value);
uint8_t encodeUtf8(uint8_t* buf, int value);
uint32_t getByteNumOfDecodeUtf8(uint8_t byte);
int decodeUtf8(const uint8_t* bytePtr, uint32_t length);
#endif