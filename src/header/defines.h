#pragma once
#include<stdlib.h>
#include<string.h>
#include "file.h"
#define ORDER_SUM 8
typedef enum Syntax_Cmd{
    FTP_get,
    FTP_put,
    FTP_delete,
    FTP_ls,
    FTP_cd,
    FTP_mkdir,
    FTP_pwd,
    FTP_quit,
    FTP_error
}Syntax_Cmd;
enum Msg_Data_Type{
    Control,
    Data
};

#pragma pack(1) //1字节对齐
typedef struct MsgHeader
{
    Syntax_Cmd s_cmd;
    enum Msg_Data_Type MsgType;
    int data_size;
    char data[CACHE_SIZE];
    bool last;
    bool error;
}MsgHeader;
#pragma pack()

static const char Otable[8][9]={"get ","put ","delete ","ls","cd ","mkdir ","pwd","quit"};
static const int OtableLen[8]={4, 4, 7, 2, 3, 6, 3, 4};
static const Syntax_Cmd OtableMap[9] = {
    FTP_get,
    FTP_put,
    FTP_delete,
    FTP_ls,
    FTP_cd,
    FTP_mkdir,
    FTP_pwd,
    FTP_quit,
    FTP_error
};
Syntax_Cmd decode_in(const char*argv){
    Syntax_Cmd res = FTP_error;
    int i;
    for (i = 0; i < 8; i++)
    {
        if(strncmp(argv,Otable[i],OtableLen[i])==0){
            res = OtableMap[i];
        }
    }
    return res;
}

static const int ParamNum[8] = {2,2,2,1,2,2,1,1};

// 返回参数数量
int check_param(char *str, int site[16][2],int length)
{   
    int start[256] = {0};
    int end[256] = {0};
    int start_p = 0;
    int end_p = 0;
    if (str[0] != ' ')
    {
        start[start_p++] = 0;
    }
    int i;
    for (i = 1; i < length; i++)
    {
        if (str[i] != ' ' && str[i - 1] == ' ')
        {
            start[start_p++] = i;
        }
        if ((str[i] == ' ' || str[i] == '\0') && str[i - 1] != ' ' && str[i - 1] != '\0')
        {
            end[end_p++] = i - 1;
        }
    }
    if (end_p)
    {
        for (i = 0; i < end_p; i++)
        {
            site[i][0] = start[i];
            site[i][1] = end[i];
        }
    }
    return end_p;
}
// 检查参数
Syntax_Cmd check_command(const char*argv){
    Syntax_Cmd res = FTP_error;
    int i;
    // 原本的定义有空格啥的我实在是搞不明白，把空格去掉了。但是上面的我没敢改，因为在别的地方用到了，
    // 但是这样会不会有一个问题，就是如果command后面多敲几个空格，最后读取到的参数实际上最前面是带空格的？
    // 还有就是不能只比较前n字节，否则如果输入cdc a就不会报错
    static const char Otable[8][9]={"get","put","delete","ls","cd","mkdir","pwd","quit"};
    static const int OtableLen[8]={3, 3, 6, 2, 2, 5, 3, 4};
    for (i = 0; i < 8; i++)
    {
        if(strcmp(argv,Otable[i])==0){
            res = OtableMap[i];
        }
    }
    return res;
}

