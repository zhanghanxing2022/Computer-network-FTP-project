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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define SERVER_PORT 5050
#define SERVER_IP "127.0.0.1"
#define QUEUE_SIZE 5

#include"header\defines.h"


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

    char sendbuf[sizeof(struct MsgHeader)+1];     //申请一个发送数据缓存区
    char recvbuf[sizeof(struct MsgHeader)+1];     //申请一个接收数据缓存区
    memset(sendbuf,0,sizeof(sendbuf));
    memset(recvbuf,0,sizeof(recvbuf));
    while (1)
    {
        printf("Ser:>");
        scanf("%[^\n]",sendbuf);
        fflush(stdin);
        if(strncmp(sendbuf, "quit", 4) ==0)
            break;
        else send(sockConn, sendbuf, strlen(sendbuf)+1, 0);

        printf("Cli:> ");
        //Control Connection receive
        recv(sockConn, recvbuf, sizeof(struct MsgHeader)+1, 0);
        struct MsgHeader* ControlMsg=(struct MsgHeader*) recvbuf;
        //printf("receive Control connection\n");
        /*
        struct MsgHeader Control_recvMsg;
        Control_recvMsg.last=1;
        send(sockConn, (char*)&Control_recvMsg, sizeof(struct MsgHeader)+1, 0);
        */
        struct Readbolck block;
        memset(&block,0,sizeof(block));
        switch(ControlMsg->Cmdtype){
            case FTP_put:
                //get filename
                strncpy(block.filepath,"ServerFile/",11);
                strncpy(block.filepath+11,ControlMsg->data+4,ControlMsg->data_size-4);
                //printf("filepath:%s\n",block.filepath);
                //Data Receive
                struct MsgHeader* DataMsg;
                do{
                    recv(sockConn, recvbuf, sizeof(struct MsgHeader)+1, 0);
                    DataMsg=(struct MsgHeader*) recvbuf;
                    block.cache=DataMsg->data;
                    put_in_file(&block,DataMsg->data_size);
                }while(DataMsg->last==0);
                printf("\n");
                break;
            case FTP_mkdir:
                strncpy(block.filepath,"ServerFile/",11);
                strncpy(block.filepath+11,ControlMsg->data+6,ControlMsg->data_size-6);
                if(_access(block.filepath,0)==-1){
                    mkdir(block.filepath);
                }
                printf("\n");
                break;
        }

        


        
        

        
    }
    close(sockSer);
    return 0;
}