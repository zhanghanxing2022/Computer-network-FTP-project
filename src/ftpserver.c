#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#elif defined __APPLE__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <sys/types.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define SERVER_PORT 5050
#define SERVER_IP "127.0.0.1"
#define QUEUE_SIZE 5
const char Serpath[] = "ServerFile";
#include "header/defines.h"
#include <dirent.h>

// 当前路径
char curpath[256];
// 用戶當前路徑
char client_current_path[256] = "root";

int main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        printf("Error: Unable to start WSAS\n");
    }
#endif
    int sockSer;
    sockSer = socket(AF_INET, SOCK_STREAM, 0);
    if (sockSer == -1)
    {
        perror("socket");
    }
#ifdef _WIN32
    char yes[256] = "1";
#elif defined __APPLE__
    int temp = 1;
    int *yes = &temp;
#endif

    if (setsockopt(sockSer, SOL_SOCKET, SO_REUSEADDR, yes, sizeof(int)) == -1)
    {
        perror("socket");
    }

    struct sockaddr_in addrSer, addrCli;            //创建一个记录地址信息的结构体
    addrSer.sin_family = AF_INET;                   //创建一个记录地址信息的结构体
    addrSer.sin_port = htons(SERVER_PORT);          //设置地址结构体中的端口号
    addrSer.sin_addr.s_addr = inet_addr(SERVER_IP); //设置其中的服务器ip

    socklen_t addrlen = sizeof(struct sockaddr);
    int res = bind(sockSer, (struct sockaddr *)&addrSer, addrlen);
    if (res == -1)
    {
        perror("bind");
    }

    listen(sockSer, QUEUE_SIZE);

    printf("Server Wait Client Accept………\n ");

    int sockConn = accept(sockSer, (struct sockaddr *)&addrCli, &addrlen);
    if (sockConn == -1)
        perror("accept");
    else
    {
        printf("Server Accept Client OK.\n");
        printf("Client IP:> %s\n", inet_ntoa(addrCli.sin_addr));
        printf("Client Port:> %d\n", ntohs(addrCli.sin_port));
    }

    // char sendbuf[sizeof(struct MsgHeader)+1];     //申请一个发送数据缓存区
    // struct MsgHeader* send_msg=(struct MsgHeader*)sendbuf;
    char recvbuf[sizeof(struct MsgHeader) + 1]; //申请一个接收数据缓存区
    // memset(sendbuf,0,sizeof(sendbuf));
    memset(recvbuf, 0, sizeof(recvbuf));
    struct MsgHeader SendMsg;
    // 改成全局变量
    // char curpath[256];
    strncpy(curpath, Serpath, sizeof(Serpath));
    while (1)
    {
        fflush(stdin);

        printf("Cli:> ");
        // Control Connection receive
        recv(sockConn, recvbuf, sizeof(struct MsgHeader) + 1, 0);
        struct MsgHeader *ControlMsg = (struct MsgHeader *)recvbuf;
        // Control Connection response
        // struct MsgHeader recvMsg;
        // recvMsg.last=1;
        // send(sockConn, (char*)&recvMsg, sizeof(struct MsgHeader)+1, 0);

        // Control Connection response
        memset(&SendMsg, 0, sizeof(SendMsg));
        SendMsg.s_cmd = ControlMsg->s_cmd;
        SendMsg.last = true;
        SendMsg.error = false;

        struct Readbolck block;
        memset(&block, 0, sizeof(block));
        switch (ControlMsg->s_cmd)
        {
        case FTP_get:
            // Control msg receviced
            SendMsg.data_size = 0;
            send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);

            // data msg
            memset(SendMsg.data, 0, sizeof(SendMsg.data));
            SendMsg.MsgType = Data;
            SendMsg.s_cmd = FTP_get;
            block.cache = SendMsg.data;
            block.method = BY_BIT;
            strncpy(block.filepath, curpath, strlen(curpath));
            strcat(block.filepath, "/");
            strcat(block.filepath, ControlMsg->data);
            printf("get %s\n", block.filepath);
            if(file_type(block.filepath)!=A_FILE){
                SendMsg.error = true;
                SendMsg.last = true;
                memset(SendMsg.data,0,sizeof(SendMsg.data));
                strcat(SendMsg.data, file_type(block.filepath)==A_DIR?  "File not Found.Only find a dir.":"File not Found.");
                send(sockConn, (char *)&SendMsg, sizeof(MsgHeader) + 1, 0);
                break;
            }
            while (block.lst == false && block.error == false)
            {
                read_from_file(&block, CACHE_SIZE);
                SendMsg.data_size = block.cur_size;
                SendMsg.last = block.lst;
                do
                {
                    send(sockConn, (char *)&SendMsg, sizeof(MsgHeader) + 1, 0);
                    // recv(sockConn, recvbuf, sizeof(struct MsgHeader) + 1, 0);
                } while (ControlMsg->error == true);
            }
            break;
        case FTP_put:
            SendMsg.data_size = 0;
            SendMsg.error = false;
            send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);

            // get filename
            strncpy(block.filepath, curpath, strlen(curpath));
            strcat(block.filepath, "/");
            strcat(block.filepath, ControlMsg->data);

            //删除已经存在的文件
            remove(block.filepath);

            // Data Receive
            struct MsgHeader *DataMsg;
            DataMsg = (struct MsgHeader *)recvbuf;
            DataMsg->last = false;

            memset(&SendMsg.data, 0, SendMsg.data_size);
            SendMsg.data_size = 0;
            SendMsg.error = false;

            while (DataMsg->last == false)
            {
                //首先确定DataMsg->error==false
                recv(sockConn, recvbuf, sizeof(struct MsgHeader) + 1, 0);
                   
                block.cache = DataMsg->data;
                block.method =BY_BIT;
                put_in_file(&block, DataMsg->data_size);
            }
            printf("\n");
            break;
        case FTP_delete:
        {
            // filename:ControlMsg->data
            char delete_path[256];
            memset(delete_path, 0, sizeof(delete_path));
            strcpy(delete_path, curpath);
            strcat(delete_path, "/");
            strcat(delete_path, ControlMsg->data);
            if (remove(delete_path) == -1)
            {
                printf("error\n");
                memcpy(SendMsg.data, "fail to delete", 15);
                SendMsg.data_size = 15;
                SendMsg.error = true;
            }
            else
            {
                SendMsg.error = false;
            }
            send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);
            break;
        }
        case FTP_ls:
        {
            // Control msg
            SendMsg.data_size = 0;
            send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);

            // Data msg
            SendMsg.s_cmd = FTP_ls;
            SendMsg.MsgType = Data;

            int i = 0;
            int filesize = 0;
            DIR *dir = NULL;
            char buf[256];
            memset(buf, 0, 256);
            struct dirent *entry;
            dir = opendir(curpath);
            while ((entry = readdir(dir)))
            {
                i++;
                strcat(buf, (char *)(entry->d_name));
                strcat(buf, "\t");
                filesize += sizeof(entry->d_name) + 1;
            }
            memcpy(SendMsg.data, buf, sizeof(buf));
            printf("%s\n", SendMsg.data);
            send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);
            closedir(dir);
            break;
        }
        case FTP_cd:
        {
            DIR *dir;
            if ((dir = opendir(curpath)) == NULL)
            {
                printf("file does not exit");
                SendMsg.error = true;
                memcpy(SendMsg.data, "curpath error", 14);
                SendMsg.data_size = 14;
                send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);
            }
            else
            {
                int flag = 0;
                struct dirent *entry;
                char *filename = ControlMsg->data;
                while ((entry = readdir(dir))!=NULL)
                {
                    if (strcmp(filename, entry->d_name) == 0)
                    {
                        flag = 1;
                        if (strcmp(filename, "..") == 0)
                        {
                            // change to the parent directory
                            // char copy_path[128];
                            // memset(copy_path, 0, sizeof(copy_path));
                            // strcpy(copy_path, curpath);
                            char *ptr = strrchr(curpath, '/');
                            if (ptr == NULL)
                            {
                                SendMsg.error = false;
                                strcpy(SendMsg.data, client_current_path);
                                printf("No parent directory\n");
                                break;
                            }
                            int len = ptr - curpath;
                            memset(ptr, 0, sizeof(curpath) - len);

                            ptr = strrchr(client_current_path, '/');
                            if (ptr == NULL)
                            {
                                printf("No parent directory\n");
                                break;
                            }
                            len = ptr - client_current_path;
                            memset(ptr, 0, sizeof(client_current_path) - len);
                            // strncpy(curpath, copy_path, len);

                            /*memset(copy_path, 0, sizeof(copy_path));
                            strcpy(copy_path, client_current_path);
                            len = ptr - copy_path;
                            memset(client_current_path, 0, sizeof(client_current_path));
                            strncpy(client_current_path, copy_path, len);*/
                        }
                        else if (strcmp(filename, ".") == 0)
                        {
                            // do nothing
                        }
                        else
                        {
                            strcat(curpath, "/");
                            strcat(curpath, filename);
                            strcat(client_current_path, "/");
                            strcat(client_current_path, filename);
                        }
                        printf("curent path:%s\n", curpath);
                        SendMsg.error = false;
                        strcpy(SendMsg.data, client_current_path);
                        break;
                    }
                }
                // 文件夾不存在
                if (!flag)
                {
                    printf("No such path\n");

                    SendMsg.error = true;
                    memcpy(SendMsg.data, "no such path", 13);
                    SendMsg.data_size = 13;
                }
                send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);
                closedir(dir);
            }
            break;
        }
        case FTP_mkdir:
            strncpy(block.filepath, curpath, strlen(curpath));
            // strncpy(block.filepath,"ServerFile/",11);
            strncpy(block.filepath + strlen(curpath), "/", 1);
            strncpy(block.filepath + strlen(curpath) + 1, ControlMsg->data, ControlMsg->data_size);
            #ifdef _WIN32 
            if (_access(block.filepath, 0) == -1)
            #elif defined __APPLE__
            if (access(block.filepath, 0) == -1)
            #endif
            {
                #ifdef _WIN32
                if (mkdir(block.filepath) == -1)
                #elif defined __APPLE__
                if (mkdir(block.filepath,0777) == -1)
                #endif
                {
                    SendMsg.error = true;
                    memcpy(SendMsg.data, "fail to make a new directory", 29);
                    SendMsg.data_size = 29;
                    printf("fail to make a new directory\n");
                }
                else
                    printf("create directory %s\n", ControlMsg->data);
            }
            else
            {
                SendMsg.error = true;
                memcpy(SendMsg.data, "the directory exists", 21);
                SendMsg.data_size = 21;
                printf("the directory exists\n");
            }
            send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);
            printf("\n");
            break;
        case FTP_pwd:
            // Control receive
            SendMsg.error = false;
            send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);

            // send working directory to client
            SendMsg.s_cmd = FTP_pwd;
            SendMsg.MsgType = Data;
            memcpy(SendMsg.data, curpath, sizeof(curpath));
            SendMsg.data_size = sizeof(curpath);
            send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);
            break;
        case FTP_quit:
            send(sockConn, (char *)&SendMsg, sizeof(struct MsgHeader) + 1, 0);
            printf("Bye!\n");
            sleep(1);
            exit(0);
            break;
        default:
            break;
        }
    }
    close(sockSer);
    return 0;
}
