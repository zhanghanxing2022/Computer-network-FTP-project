#ifdef _WIN32
    #include<WinSock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib,"ws2_32.lib")
#elif defined __APPLE__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif
#include <sys/types.h>
#include "header/defines.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define SERVER_PORT 5050
#define SERVER_IP "127.0.0.1"
#define QUEUE_SIZE 5
const char Serpath[] = "./ServerFile/";


int main(int argc, char *argv[]){
#ifdef _WIN32
    WSADATA wsaData;
    if(0 !=WSAStartup(MAKEWORD(2,2), &wsaData)){
        printf("Error: Unable to start WSAS\n");
    }
#endif
    int sockSer;
    sockSer = socket(AF_INET, SOCK_STREAM, 0);
    if(sockSer == -1){
        perror("socket");
    }
#ifdef _WIN32
    char yes[256] = "1";
#elif defined __APPLE__
    int temp = 1;
    int* yes = &temp;
#endif

    if(setsockopt(sockSer, SOL_SOCKET, SO_REUSEADDR,yes,sizeof(int))==-1){
        perror("socket");
    }

    struct sockaddr_in addrSer,addrCli; //创建一个记录地址信息的结构体
    addrSer.sin_family = AF_INET;//创建一个记录地址信息的结构体
    addrSer.sin_port = htons(SERVER_PORT);//设置地址结构体中的端口号
    addrSer.sin_addr.s_addr = inet_addr(SERVER_IP);//设置其中的服务器ip

    socklen_t addrlen = sizeof(struct sockaddr);
    int res = bind(sockSer,(struct sockaddr*)&addrSer,addrlen);
    if(res == -1){
        perror("bind");
    }

    listen(sockSer, QUEUE_SIZE);

    printf("Server Wait Client Accept………\n ");

    int sockConn = accept(sockSer, (struct sockaddr*)&addrCli, &addrlen);
    if(sockConn == -1)
        perror("accept");
    else{
        printf("Server Accept Client OK.\n");
        printf("Client IP:> %s\n", inet_ntoa(addrCli.sin_addr));
        printf("Client Port:> %d\n", ntohs(addrCli.sin_port));
    }

    char sendbuf[sizeof(MsgHeader)+1];     //申请一个发送数据缓存区
    char recvbuf[sizeof(MsgHeader)+1];     //申请一个接收数据缓存区
    memset(sendbuf,0,sizeof(sendbuf));
    memset(recvbuf,0,sizeof(recvbuf));
    MsgHeader *SendMsg = (MsgHeader*)sendbuf;
    MsgHeader *RecvMsg = (MsgHeader*)recvbuf;
    Readbolck block;
    while (1)
    {
        recv(sockConn, recvbuf, sizeof(recvbuf)+1, 0);
        printf("Cli>%s\n",RecvMsg->data);
        switch (RecvMsg->s_cmd)
        {
        case FTP_get:
            memset(&block,0,sizeof(block));
            block.cache = SendMsg->data;
            memset(SendMsg->data,0,sizeof(SendMsg->data));
            block.method = BY_BIT;
            strncpy(block.filepath,Serpath,strlen(Serpath));
            strcat(block.filepath, RecvMsg->data);
            printf("get %s\n",block.filepath);
            while (block.lst==false&&block.error==false)
            {
                read_from_file(&block,CACHE_SIZE);
                printf("Ser>data_size:%d\n",block.data_size);
                SendMsg->data_size = block.data_size;
                SendMsg->last = block.lst;
                printf("Ser>len:%d\n",block.data_size);
                send(sockConn, SendMsg, sizeof(MsgHeader)+1, 0);
            }
            break;
        
        case FTP_put:
            memset(&block,0,sizeof(block));
            block.method = BY_BIT;
            //get filename
            strncpy(block.filepath,Serpath,sizeof(Serpath));
            strcat(block.filepath,RecvMsg->data);
            //printf("filepath:%s\n",block.filepath);
            //Data Receive
            struct MsgHeader* DataMsg;
            do{
                recv(sockConn, recvbuf, sizeof(struct MsgHeader)+1, 0);
                DataMsg=(struct MsgHeader*) recvbuf;
                block.cache=DataMsg->data;
                block.data_size = DataMsg->data_size;
                put_in_file(&block,0);
            }while(DataMsg->last==0);
            printf("\n");
            break;
        case FTP_mkdir:
            memset(&block,0,sizeof(block));
            strncpy(block.filepath,Serpath,sizeof(Serpath));
            strcat(block.filepath,RecvMsg->data);
            printf("Ser>mkdir %s",block.filepath);
            if(file_type(block.filepath) == NOT_FOUND){
                mkdir(block.filepath,0777);
            }
            printf("\n");
            break;
        default:
            break;
        }
        
    }
    
    
    
    
    printf("Cli:> %s\n", recvbuf);

    
    close(sockSer);
    return 0;
}