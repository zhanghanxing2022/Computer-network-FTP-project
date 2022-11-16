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
    for (int i = 0; i < 8; i++)
    {
        if(strncmp(argv,Otable[i],OtableLen[i])==0){
            res = OtableMap[i];
        }
    }
    return res;
}
