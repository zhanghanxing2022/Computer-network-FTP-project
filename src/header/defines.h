#pragma once
#include<stdlib.h>
#include<string.h>
#include "file.h"
#define ORDER_SUM 8
enum Syntax_Cmd{
    FTP_get,
    FTP_put,
    FTP_delete,
    FTP_ls,
    FTP_cd,
    FTP_mkdir,
    FTP_pwd,
    FTP_quit
};
typedef struct MsgHeader
{
    int a;
}MsgHeader;
typedef struct 
{
    /* data */
};

MsgHeader ftpCli_get(const char* argv, int len);
MsgHeader ftpCli_put(const char* argv,int len);
MsgHeader ftpCli_delete(const char* argv,int len);
MsgHeader ftpCli_ls(const char* argv=nullptr,int len = 0);
MsgHeader ftpCli_cd(const char* argv,int len);
MsgHeader ftpCli_mkdir(const char* argv,int len);
MsgHeader ftpCli_pwd(const char* argv=nullptr,int len=0);
MsgHeader ftpCli_quit(const char* argv=nullptr,int len=0);

static MsgHeader(*syntax_tableCli[ORDER_SUM])(const char*, int)={
    &ftpCli_get,
    &ftpCli_put,
    &ftpCli_delete,
    &ftpCli_ls,
    &ftpCli_cd,
    &ftpCli_mkdir,
    &ftpCli_pwd,
    &ftpCli_quit
};

MsgHeader ftpSer_get(const char* argv, int len);
MsgHeader ftpSer_put(const char* argv,int len);
MsgHeader ftpSer_delete(const char* argv,int len);
MsgHeader ftpSer_ls(const char* argv=nullptr,int len = 0);
MsgHeader ftpSer_cd(const char* argv,int len);
MsgHeader ftpSer_mkdir(const char* argv,int len);
MsgHeader ftpSer_pwd(const char* argv=nullptr,int len=0);
MsgHeader ftpSer_quit(const char* argv=nullptr,int len=0);

static MsgHeader(*syntax_tableSer[ORDER_SUM])(const char*, int)={
    &ftpSer_get,
    &ftpSer_put,
    &ftpSer_delete,
    &ftpSer_ls,
    &ftpSer_cd,
    &ftpSer_mkdir,
    &ftpSer_pwd,
    &ftpSer_quit
};


MsgHeader ftpCli_get(const char* argv, int len){
    //get (get [remote_filename]) -- Copy file with the name [remote_filename]  from remote directory to local directory.
    char *buf = (char*)calloc(255,sizeof(char));
    Readbolck block;
    memset(&block,0,sizeof(block));
    block.cache=buf;
    memcpy(block.filepath, argv, len*sizeof(char));
    
    
}
MsgHeader ftpSer_get(const char* argv, int len){
    char *buf = (char*)calloc(CACHE_SIZE, sizeof(char));
    Readbolck block;
    memset(&block,0,sizeof(block));
    block.cache=buf;
    memcpy(block.filepath, argv, len*sizeof(char));
    while(block.lst == false)
    {
        read_from_file(&block, len);
    }
}