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
    MsgHeader *SendHeader = (MsgHeader*)sendbuf;
    MsgHeader *RecvHeader = (MsgHeader*)recvbuf;
    while (1)
    {
        recv(sockConn, recvbuf, sizeof(recvbuf)+1, 0);
        printf("Cli>%s\n",RecvHeader->data);
        if(RecvHeader->s_cmd == FTP_get){
            Readbolck sendblock;
            memset(&sendblock,0,sizeof(sendblock));
            sendblock.cache = SendHeader->data;
            memset(SendHeader->data,0,sizeof(SendHeader->data));
            sendblock.method = BY_ASCII;
            strncpy(sendblock.filepath,Serpath,strlen(Serpath));
            strcat(sendblock.filepath, RecvHeader->data);
            printf("get %s\n",sendblock.filepath);
            while (sendblock.lst==false&&sendblock.error==false)
            {
                read_from_file(&sendblock,CACHE_SIZE-100);
                // printf("%s\n",SendHeader->data);
                printf("%d\n",sendblock.cur_size);
                SendHeader->cur_size = sendblock.cur_size;
                SendHeader->last = sendblock.lst;
                printf("len:%d\n",sendblock.cur_size);
                send(sockConn, SendHeader, sizeof(MsgHeader)+1, 0);
                recv(sockConn, recvbuf, sizeof(recvbuf), 0);
            }
            
        }
    }
    
    
    
    
    printf("Cli:> %s\n", recvbuf);

    
    close(sockSer);
    return 0;
}