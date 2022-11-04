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
enum Msg_Data_Type{
    Control,
    Data
};

#pragma pack(1) //1字节对齐
typedef struct MsgHeader
{
    enum Msg_Data_Type MsgType;
    enum Syntax_Cmd Cmdtype;
    int data_size;
    char data[CACHE_SIZE];
    bool last;
}MsgHeader;
#pragma pack()

//MsgHeader ftpCli_get(const char* argv, int len);
//MsgHeader ftpCli_put(const char* argv,int len);
//MsgHeader ftpCli_delete(const char* argv,int len);
//MsgHeader ftpCli_ls(const char* argv=nullptr,int len = 0);
//MsgHeader ftpCli_cd(const char* argv,int len);
//MsgHeader ftpCli_mkdir(const char* argv,int len);
//MsgHeader ftpCli_pwd(const char* argv=nullptr,int len=0);
//MsgHeader ftpCli_quit(const char* argv=nullptr,int len=0);
/*static MsgHeader(*syntax_tableCli[ORDER_SUM])(const char*, int)={
    //&ftpCli_get,
    &ftpCli_put,
    //&ftpCli_delete,
    //&ftpCli_ls,
    //&ftpCli_cd,
    &ftpCli_mkdir
    //&ftpCli_pwd,
    //&ftpCli_quit
};*/


//MsgHeader ftpSer_get(const char* argv, int len);
//MsgHeader ftpSer_put(const char* argv,int len);
//MsgHeader ftpSer_delete(const char* argv,int len);
//MsgHeader ftpSer_ls(const char* argv=nullptr,int len = 0);
//MsgHeader ftpSer_cd(const char* argv,int len);
//MsgHeader ftpSer_mkdir(const char* argv,int len);
//MsgHeader ftpSer_pwd(const char* argv=nullptr,int len=0);
//MsgHeader ftpSer_quit(const char* argv=nullptr,int len=0);

/*static MsgHeader(*syntax_tableSer[ORDER_SUM])(const char*, int)={
    //&ftpSer_get,
    &ftpCli_put,
    //&ftpCli_delete,
    //&ftpCli_ls,
    //&ftpCli_cd,
    &ftpCli_mkdir
    //&ftpCli_pwd,
    //&ftpCli_quit
};*/

/*
MsgHeader ftpCli_put(const char* argv,int len){
    //put(put [local_filename]) -- Copy file with the name [local_filename]  from local directory to remote directory.
    char *buf = (char*)calloc(CACHE_SIZE, sizeof(char));
    
    //read from file and store in block
    struct Readbolck block;
    memset(&block,0,sizeof(block));
    block.cache=buf;
    memcpy(block.filepath, argv, len*sizeof(char));
    while(block.error==false&&block.lst == false)
    {
        //printf("in reading cycle\n");
        read_from_file(&block, len);
    }
    
    //put into MsgHeader msg
    struct MsgHeader msg;
    msg.MsgType=Data;
    msg.Cmdtype=FTP_put;
    strcpy(msg.Msgbuf.name,argv);
    strcpy(msg.Msgbuf.buf,block.cache);
    msg.Msgbuf.buffer_size=block.cur_size;
    return msg;
}
MsgHeader ftpSer_put(const char* argv, int len){
    struct MsgHeader* msg=(struct MsgHeader*) argv;

    //change directory
    strncpy(msg->Msgbuf.name,"Server",6);

    Readbolck block;
    memset(&block,0,sizeof(block));
    block.cache=msg->Msgbuf.buf;
    memcpy(block.filepath, msg->Msgbuf.name, NAME_SIZE);
    put_in_file(&block,msg->Msgbuf.buffer_size);
}

MsgHeader ftpCli_mkdir(const char* argv,int len){
    //mkdir(mkdir[remote_directory_name])
    MsgHeader msg;
    msg.type=FTP_mkdir;
    strcpy(msg.Msgbuf.name,argv);
    return msg;
}
MsgHeader ftpSer_mkdir(const char* argv,int len){
    struct MsgHeader* msg=(struct MsgHeader*) argv;
    if(_access(msg->Msgbuf.name,0)==-1){
        char filepath[NAME_SIZE];
        strcpy(filepath,"ServerFile/");
        strcat(filepath, msg->Msgbuf.name);
        mkdir(filepath);
    }
}
*/
