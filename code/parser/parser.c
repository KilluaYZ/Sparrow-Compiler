#include"parser.h"
#include<stdio.h>
#include<stdlib.h>
#include"common.h"
#include"utils.h"
#include"unicodeUtf8.h"
#include<string.h>
#include<ctype.h>
#include"obj_string.h"


//关键字结构
struct keywordToken{
    char*       keyword;
    uint8_t     length;
    TokenType   token;
};

//关键字查找表
struct keywordToken keywordsToken[] = {
    {"var",         3,      TOKEN_VAR},
    {"fun",         3,      TOKEN_FUN},
    {"if",          2,      TOKEN_IF},
    {"else",        4,      TOKEN_ELSE},
    {"true",        4,      TOKEN_TRUE},
    {"false",       5,      TOKEN_FALSE},
    {"while",       5,      TOKEN_WHILE},
    {"for",         3,      TOKEN_FOR},
    {"break",       5,      TOKEN_BREAK},
    {"continue",    8,      TOKEN_CONTINUE},
    {"return",      6,      TOKEN_RETURN},
    {"null",        4,      TOKEN_NULL},
    {"class",       5,      TOKEN_CLASS},
    {"is",          2,      TOKEN_IS},
    {"static",      6,      TOKEN_STATIC},
    {"this",        4,      TOKEN_THIS},
    {"super",       5,      TOKEN_SUPER},
    {"import",      6,      TOKEN_IMPORT},
    {NULL,          0,      TOKEN_UNKNOWN}
};

//判断start是否为关键字并返回相应的token
static TokenType idOrkeyword(const char* start, uint32_t length){
    uint32_t idx = 0;
    while(keywordsToken[idx].keyword != NULL){
        if(keywordsToken[idx].length == length && memcmp(keywordsToken[idx].keyword,start,length) == 0){
            return keywordsToken[idx].token;
        }
        idx++;
    }
    return TOKEN_ID;
}

//向前看一个字符
char lookAheadChar(Parser* parser){
    return *parser->nextCharPtr;
}

//获取下一个字符
static void getNextChar(Parser* parser){
    parser->curChar = *parser->nextCharPtr++;
}

//查看下一个字符是否为期望的，如果是就读进来，返回true，否则返回false
static bool matchNextChar(Parser* parser, char expectedChar){
    if(lookAheadChar(parser) == expectedChar){
        getNextChar(parser);
        return true;
    }
    return false;
}

//跳过连续的空白字符
static void skipBlanks(Parser* parser){
    while(isspace(parser->curChar)){
        if(parser->curChar == '\n'){
            parser->curToken.lineNo++;
        }
        getNextChar(parser);
    }
}

//解析标识符，在getNextToken函数中调用
static void parserId(Parser* parser, TokenType type){
    while(isalnum(parser->curChar) || parser->curChar=='_'){
        getNextChar(parser);
    }

    //nextCharPtr会指向第1个不合法字符的下一个字符，因此-1
    uint32_t length = (uint32_t)(parser->nextCharPtr - parser->curToken.start - 1);
    if(type != TOKEN_UNKNOWN){
        parser->curToken.type = type;
    }else{
        parser->curToken.type = idOrkeyword(parser->curToken.start,length);
    }

    parser->curToken.length = length;
}

//解析十六进制数字
static void parseHexNum(Parser* parser) {
   while (isxdigit(parser->curChar)) {
      getNextChar(parser);
   }
}

//解析十进制数字
static void parseDecNum(Parser* parser) {
   while (isdigit(parser->curChar)) {
      getNextChar(parser);
   }

   //若有小数点
   if (parser->curChar == '.' && isdigit(lookAheadChar(parser))) {
      getNextChar(parser);
      while (isdigit(parser->curChar)) { //解析小数点之后的数字
	 getNextChar(parser);
      }
   }
}

//解析八进制
static void parseOctNum(Parser* parser) {
   while(parser->curChar >= '0' && parser->curChar < '8') {
      getNextChar(parser);
   } 
}

//解析八进制 十进制 十六进制 仅支持前缀形式,后缀形式不支持
static void parseNum(Parser* parser) {
   //十六进制的0x前缀
   if (parser->curChar == '0' && matchNextChar(parser, 'x')) {
      getNextChar(parser);  //跳过'x'
      parseHexNum(parser);   //解析十六进制数字
      parser->curToken.value = 
	 NUM_TO_VALUE(strtol(parser->curToken.start, NULL, 16));
   } else if (parser->curChar == '0' 
	 && isdigit(lookAheadChar(parser))) {  // 八进制
      parseOctNum(parser);
      parser->curToken.value = 
	 NUM_TO_VALUE(strtol(parser->curToken.start, NULL, 8));
   } else {	  //解析十进制
      parseDecNum(parser);
      parser->curToken.value = NUM_TO_VALUE(strtod(parser->curToken.start, NULL));
   }
   //nextCharPtr会指向第1个不合法字符的下一个字符,因此-1
   parser->curToken.length = 
      (uint32_t)(parser->nextCharPtr - parser->curToken.start - 1);
   parser->curToken.type = TOKEN_NUM;
}


//解析unicode码点
static void parseUnicodeCodePoint(Parser* parser,ByteBuffer* buf){
    uint32_t idx = 0;
    int value = 0;
    uint8_t digit = 0;

    //获取数值，u后面跟着4位十六进制数字
    while(idx++ < 4){
        getNextChar(parser);
        if(parser->curChar == '\0'){
            LEX_ERROR(parser,"unterminated unicode!");
        }
        if(parser->curChar >= '0' && parser->curChar <= '9'){
            digit = parser->curChar - '0';
        }else if(parser->curChar >= 'a' && parser->curChar <= 'f'){
            digit = parser->curChar - 'a' + 10;
        }else if(parser->curChar >= 'A' && parser->curChar <= 'F'){
            digit = parser->curChar - 'A' + 10;
        }else {
            LEX_ERROR(parser,"invalid unicode!");
        }
        value = value * 16 | digit;
    }

    uint32_t byteNum = getByteNumOfEncodeUtf8(value);
    ASSERT(byteNum != 0,"utf8 encode bytes should be between 1 and 4!");

    //为代码通用，下面会直接写buf0>datas，在此先写入byteNum个0，以保证事先有byteNum个空间
    ByteBufferFillWrite(parser->vm,buf,0,byteNum);

    //把value编写为utf-8后写入缓冲区buf
    encodeUtf8(buf->datas+buf->count-byteNum,value);
}

//解析字符串
static void parseString(Parser* parser){
    ByteBuffer str;
    ByteBufferInit(&str);
    while(true){
        getNextChar(parser);
        if(parser->curChar == '\0'){
            LEX_ERROR(parser,"unterminated string!");
        }

        //处理字符串
        if(parser->curChar == '"'){
            parser->curToken.type = TOKEN_STRING;
            break;
        }

        //处理内嵌表达式
        if(parser->curChar == '%'){
            if(!matchNextChar(parser,'(')){
                LEX_ERROR(parser," '%' should followed by '(' !");
            }
            if(parser->interpolationExpectRightParenNum > 0){
                COMPILE_ERROR(parser,"sorry, I don't support nest interpolate expression!");
            }
            parser->interpolationExpectRightParenNum = 1;
            parser->curToken.type = TOKEN_INTERPOLATION;
            break;
        }

        //处理转义字符
        if(parser->curChar == '\\'){
            getNextChar(parser);
            switch(parser->curChar){
                case '0':
                    ByteBufferAdd(parser->vm,&str,'\0');
                    break;
                case 'a':
                    ByteBufferAdd(parser->vm,&str,'\a');
                    break;
                case 'b':
                    ByteBufferAdd(parser->vm,&str,'\b');
                    break;
                case 'f':
                    ByteBufferAdd(parser->vm,&str,'\f');
                    break;
                case 'n':
                    ByteBufferAdd(parser->vm,&str,'\n');
                    break;
                case 'r':
                    ByteBufferAdd(parser->vm,&str,'\r');
                    break;
                case 't':
                    ByteBufferAdd(parser->vm,&str,'\t');
                    break;
                case 'u':
                    parseUnicodeCodePoint(parser, &str);
                    break;
                case '"':
                    ByteBufferAdd(parser->vm,&str,'"');
                    break;
                case '\\':
                    ByteBufferAdd(parser->vm,&str,'\\');
                    break;
                default:
                    LEX_ERROR(parser, "unsupport escape \\%c",parser->curChar);
                    break;
            }
        }else{
            //普通字符
            ByteBufferAdd(parser->vm, &str, parser->curChar);
        }
    }
    //用识别到的字符串新建字符串对象储存到curToken的value中
    ObjString* objString = newObjString(parser->vm,(const char*)str.datas,str.count);
    parser->curToken.value = OBJ_TO_VALUE(objString);
    ByteBufferClear(parser->vm,&str);
}

//跳过一行
static void skipAline(Parser* parser){
    getNextChar(parser);
    while(parser->curChar != '\0'){
        if(parser->curChar == '\n'){
            parser->curToken.lineNo++;
            getNextChar(parser);
            break;
        }
        getNextChar(parser);
    }
}

//跳过行注释以及区块注释
static void skipComment(Parser* parser){
    char nextChar = lookAheadChar(parser);
    if(parser->curChar == '/'){
        skipAline(parser);
    }else{
        while(nextChar != '*' && nextChar != '\0'){
            getNextChar(parser);
            if(parser->curChar == '\n'){
                parser->curToken.lineNo++;
            }
            nextChar = lookAheadChar(parser);
        }
        if(matchNextChar(parser,'*')){
            if(!matchNextChar(parser,'/')){
                //匹配*/
                LEX_ERROR(parser, "expect '/' after '*'!");
            }

            getNextChar(parser);
        }else{
            LEX_ERROR(parser,"expect '*/' before file end! ");
        }
    }
    //注释之后可能会有空白字符，所以要跳过
    skipBlanks(parser);
}

//获得下一个token，可以理解为词法分析器
void getNextToken(Parser* parser){
    parser->preToken = parser->curToken;// 记录当前token
    skipBlanks(parser); // 跳过待识别单词之前的空格
    parser->curToken.type = TOKEN_EOF;//一些初始化操作
    parser->curToken.length = 0;
    parser->curToken.start=parser->nextCharPtr - 1;//似乎可以用parser->curChar替代
    parser->curToken.value = VT_TO_VALUE(VT_UNDEFINED);
    while(parser->curChar != '\0'){
        switch(parser->curChar){
            case ',':
                parser->curToken.type = TOKEN_COMMA;
                break;
            case ':':
                parser->curToken.type = TOKEN_COLON;
                break;
            case '(':
                if(parser->interpolationExpectRightParenNum > 0){
                    parser->interpolationExpectRightParenNum++;
                }
                parser->curToken.type = TOKEN_LEFT_PAREN;
                break;
            case ')':
                if(parser->interpolationExpectRightParenNum > 0){
                    parser->interpolationExpectRightParenNum--;
                    if(parser->interpolationExpectRightParenNum == 0){
                        parseString(parser);
                        break;
                    }
                }
                parser->curToken.type = TOKEN_RIGHT_PAREN;
                break;
            case '[':
                parser->curToken.type = TOKEN_LEFT_BRACKET;
                break;
            case ']':
                parser->curToken.type = TOKEN_RIGHT_BRACKET;
                break;
            case '{':
                parser->curToken.type = TOKEN_LEFT_BRACE;
                break;
            case '}':
                parser->curToken.type = TOKEN_RIGHT_BRACE;
                break;
            case '.':
                if(matchNextChar(parser,'.')){
                    parser->curToken.type = TOKEN_DOT_DOT;
                }else{
                    parser->curToken.type = TOKEN_DOT;
                }
                break;
            case '=':
                if(matchNextChar(parser,'=')){
                    parser->curToken.type = TOKEN_EQUAL;
                }else{
                    parser->curToken.type = TOKEN_ASSIGN;
                }
                break;
            case '+':
                parser->curToken.type = TOKEN_ADD;
                break;
            case '-':
                parser->curToken.type = TOKEN_SUB;
                break;
            case '*':
                parser->curToken.type = TOKEN_MUL;
                break;
            case '/':
                //跳过注释'//' '/*'
                if(matchNextChar(parser,'/') || matchNextChar(parser,'*')){
                    skipComment(parser);
                    //重置下一个token起始地址
                    parser->curToken.start = parser->nextCharPtr - 1;
                    continue;
                }else{
                    parser->curToken.type = TOKEN_DIV;
                }
                break;
            case '%':
                parser->curToken.type = TOKEN_MOD;
                break;
            case '&':
                if(matchNextChar(parser,'&')){
                    parser->curToken.type = TOKEN_LOGIC_AND;
                }else{
                    parser->curToken.type = TOKEN_BIT_AND;
                }
                break;
            case '|':
                if(matchNextChar(parser,'|')){
                    parser->curToken.type = TOKEN_LOGIC_OR;
                }else{
                    parser->curToken.type = TOKEN_BIT_OR;
                }
                break;
            case '~':
                parser->curToken.type = TOKEN_BIT_NOT;
                break;
            case '?':
                parser->curToken.type = TOKEN_QUESTION;
                break;
            case '>':
                if(matchNextChar(parser,'=')){
                    parser->curToken.type = TOKEN_GREATE_EQUAL;
                }else if (matchNextChar(parser,'>')){
                    parser->curToken.type = TOKEN_BIT_SHIFT_RIGHT;
                }else{
                    parser->curToken.type = TOKEN_GREATE;
                }
                break;
             case '<':
                if(matchNextChar(parser,'=')){
                    parser->curToken.type = TOKEN_LESS_EQUAL;
                }else if (matchNextChar(parser,'<')){
                    parser->curToken.type = TOKEN_BIT_SHIFT_LEFT;
                }else{
                    parser->curToken.type = TOKEN_LESS;
                }
                break;
            case '!':
                if(matchNextChar(parser,'=')){
                    parser->curToken.type = TOKEN_NOT_EQUAL;
                }else{
                    parser->curToken.type = TOKEN_LOGIC_NOT;
                }
                break;
            case '"':
                parseString(parser);
                break;
            default:
                //处理变量名，数字
                //进入该分支的字符肯定是数字或变量名的首字符
                //后面会调用相应函数把其余字符一并解析
                
                //若首字符是字母或_开头的，则是变量
                if(isalpha(parser->curChar) || parser->curChar == '_'){
                    //解析变量名的其余部分
                    parserId(parser,TOKEN_UNKNOWN);
                }else if(isdigit(parser->curChar)){
                    //数字
                    parseNum(parser);
                }else{
                    if(parser->curChar == '#' && matchNextChar(parser,'!')){
                        //跳过文件头#!对解释器的指定
                        skipAline(parser);
                        //重置下一个token起始地址
                        parser->curToken.start = parser->nextCharPtr - 1;
                        continue;
                    }
                    LEX_ERROR(parser,"unsupport char: \'%c\', quit.",parser->curChar);
                }
                return;
        }
        //大部分case的出口
        parser->curToken.length = (uint32_t)(parser->nextCharPtr - parser->curToken.start);
        getNextChar(parser);
        return;
    }
}

//若当前token为expected则读入下一个token并返回true
//否则不读入token且返回false
bool matchToken(Parser* parser,TokenType expected){
    if(parser->curToken.type == expected){
        getNextToken(parser);
        return true;
    }
    return false;
}

//断言当前token为expected并读入下一个token，否则报错errMsg
void consumeCurToken(Parser* parser, TokenType expected, const char* errMsg){
    if(parser->curToken.type != expected){
        COMPILE_ERROR(parser,errMsg);
    }
    getNextToken(parser);
}

//断言下一个token为expected，否则报错errMsg
void consumeNextToken(Parser* parser, TokenType expected,const char* errMsg){
    getNextToken(parser);
    if(parser->curToken.type != expected){
        COMPILE_ERROR(parser,errMsg);
    }
}

//由于sourceCode未必来自于文件file，有可能只是个字符串
//file仅用于跟踪待编译的代码的标识符，方便报错
void initParser(VM* vm, Parser* parser, const char* file, const char* sourceCode,ObjModule* objModule){
    parser->file = file;
    parser->sourceCode = sourceCode;        //源代码
    parser->curChar = *parser->sourceCode;  //指向源代码的第一个字符
    parser->nextCharPtr = parser->sourceCode + 1;
    parser->curToken.lineNo = 1;
    parser->curToken.type = TOKEN_UNKNOWN;
    parser->curToken.start = NULL;
    parser->curToken.length = 0;
    parser->preToken = parser->curToken;
    parser->interpolationExpectRightParenNum = 0;
    parser->vm = vm;
    parser->curModule = objModule;
}

