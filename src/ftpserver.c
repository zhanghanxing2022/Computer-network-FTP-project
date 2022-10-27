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


#pragma pack(1) // 设置结构体1字节对齐**************

struct MsgHeader
{
    /* data */
};
#pragma pack()

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

    char sendbuf[256];     //申请一个发送数据缓存区
    char recvbuf[256];     //申请一个接收数据缓存区
    memset(sendbuf,0,sizeof(sendbuf));
    memset(recvbuf,0,sizeof(recvbuf));
    while (1)
    {
        printf("Ser:>");
        scanf("%[^\n]",sendbuf);
        fflush(stdin);
        if(strncmp(sendbuf, "quit", 4) ==0)
            break;
        send(sockConn, sendbuf, strlen(sendbuf)+1, 0);
        recv(sockConn, recvbuf, 256, 0);
        printf("Cli:> %s\n", recvbuf);

    }
    close(sockSer);
    return 0;
}