#include<unistd.h>
#include<stdio.h>
#include<string.h>
#ifdef _WIN32
    #include<Winsock2.h>
    #include <ws2tcpip.h>
#elif defined __APPLE__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif
 
#define SERVER_PORT  5050
#define SERVER_IP    "127.0.0.1"
 
int main(int argc, char *argv[])
{
    //创建客户端套接字号，并检测是否创建成功
    int sockCli;
    sockCli = socket(AF_INET, SOCK_STREAM, 0);
    if(sockCli == -1)
        perror("socket");
 
    //创建一个地址信息结构体，并对其内容进行设置
    struct sockaddr_in addrSer,addrCli;
    addrSer.sin_family = AF_INET;         //使用AF_INET协议族
    addrSer.sin_port = htons(SERVER_PORT);  //设置端口号
    addrSer.sin_addr.s_addr = inet_addr(SERVER_IP);   //设置服务器ip
 
    bind(sockCli,(struct sockaddr*)&addrCli, sizeof(struct sockaddr));    //将套接字地址与所创建的套接字号联系起来
 
    //创建一个与服务器的连接，并检测连接是否成功
    socklen_t addrlen = sizeof(struct sockaddr);
    int res = connect(sockCli,(struct sockaddr*)&addrSer, addrlen);
    if(res == -1)
        perror("connect");
    else
        printf("Client Connect Server OK.\n");
 
    char sendbuf[256];     //申请一个发送数据缓存区
    char recvbuf[256];     //申请一个接收数据缓存区
    memset(sendbuf,0,sizeof(sendbuf));
    memset(recvbuf,0,sizeof(recvbuf));
    while(1)
    {
        recv(sockCli, recvbuf, 256, 0);    //接收来自服务器的数据
        printf("Ser:> %s\n",recvbuf);
        printf("Cli:>");
        scanf("%[^\n]",sendbuf);
        fflush(stdin);
        if(strncmp(sendbuf,"quit", 4) == 0)    //如果客户端发送的数据为"quit"，则退出。
            break;
        send(sockCli, sendbuf, strlen(sendbuf)+1, 0);   //发送数据
    }
    close(sockCli);       //关闭套接字
    return 0;
}