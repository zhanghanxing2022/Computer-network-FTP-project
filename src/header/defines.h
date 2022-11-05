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
    FTP_error};
typedef enum MSG_DATA_TYPE
{
    DATA,
    CONTROL
} MSG_DATA_TYPE;

#pragma pack(1)
typedef struct MsgHeader
{
    Syntax_Cmd s_cmd;
    MSG_DATA_TYPE DorI;
    CODE_METRIC method;
    bool last;
    bool error;
    int data_size;
    char data [256];
}MsgHeader;
#pragma pack()

void ftpCli_get(
    const char* argv, MsgHeader*msghdr);
void ftpCli_put(
    const char* argv, MsgHeader*msghdr);
void ftpCli_delete(
    const char* argv, MsgHeader*msghdr);
void ftpCli_ls(
    const char* argv, MsgHeader*msghdr);
void ftpCli_cd(
    const char* argv, MsgHeader*msghdr);
void ftpCli_mkdir(
    const char* argv,MsgHeader*msghdr);
void ftpCli_pwd(
    const char* argv,MsgHeader*msghdr);
void ftpCli_quit(
    const char* argv,MsgHeader*msghdr);

static void(*syntax_tableCli[ORDER_SUM])(const char*,MsgHeader*)={
    &ftpCli_get,
    &ftpCli_put,
    &ftpCli_delete,
    &ftpCli_ls,
    &ftpCli_cd,
    &ftpCli_mkdir,
    &ftpCli_pwd,
    &ftpCli_quit
};

void ftpSer_get(
    const char* argv, MsgHeader*msghdr);
void ftpSer_put(
    const char* argv,MsgHeader*msghdr);
void ftpSer_delete(
    const char* argv,MsgHeader*msghdr);
void ftpSer_ls(
    const char* argv,MsgHeader*msghdr);
void ftpSer_cd(
    const char* argv,MsgHeader*msghdr);
void ftpSer_mkdir(
    const char* argv,MsgHeader*msghdr);
void ftpSer_pwd(
    const char* argv,MsgHeader*msghdr);
void ftpSer_quit(
    const char* argv,MsgHeader*msghdr);

static void(*syntax_tableSer[ORDER_SUM])(const char*, int)={
    &ftpSer_get,
    &ftpSer_put,
    &ftpSer_delete,
    &ftpSer_ls,
    &ftpSer_cd,
    &ftpSer_mkdir,
    &ftpSer_pwd,
    &ftpSer_quit
};


void ftpCli_get(const char* argv, MsgHeader*msghdr){
    //get (get [remote_filename]) -- Copy file with the name [remote_filename]  from remote directory to local directory.
    char *buf = (char*)calloc(255,sizeof(char));
    Readbolck block;
    memset(&block,0,sizeof(block));
    block.cache=buf;
    memcpy(block.filepath, argv, strlen(argv));
    
    
}
void ftpSer_get(const char* argv, MsgHeader*msghdr){
    char *buf = (char*)calloc(CACHE_SIZE, sizeof(char));
    Readbolck block;
    memset(&block,0,sizeof(block));
    block.cache=buf;
    memcpy(block.filepath, argv, strlen(argv));
    while(block.lst == false)
    {
        read_from_file(&block, strlen(argv));
    }
}

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

// MsgHeader ftpCli_mkdir(const char* argv,int len){
//     //mkdir(mkdir[remote_directory_name])
//     MsgHeader msg;
//     msg.type=FTP_mkdir;
//     strcpy(msg.Msgbuf.name,argv);
//     return msg;
// }
// MsgHeader ftpSer_mkdir(const char* argv,int len){
//     struct MsgHeader* msg=(struct MsgHeader*) argv;
//     if(_access(msg->Msgbuf.name,0)==-1){
//         char filepath[NAME_SIZE];
//         strcpy(filepath,"ServerFile/");
//         strcat(filepath, msg->Msgbuf.name);
//         mkdir(filepath);
//     }
// }
