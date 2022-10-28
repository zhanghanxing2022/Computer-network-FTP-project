#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include"header/defines.h"
#ifdef _WIN32
    #include<WinSock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib,"ws2_32.lib")
#elif defined __APPLE__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif
 
#define SERVER_PORT  5050
#define SERVER_IP    "127.0.0.1"
 
const char Clientpath[] = "./ClientFile/";
int main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
    if(0 !=WSAStartup(MAKEWORD(2,2), &wsaData)){
        printf("Error: Unable to start WSAS\n");
    }
#endif
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
 
    // char sendbuf[256];     //申请一个发送数据缓存区
    MsgHeader SendMsg ;
    char recvbuf[sizeof(MsgHeader)];     //申请一个接收数据缓存区
    memset(&SendMsg, 0 ,sizeof(MsgHeader));
    memset(recvbuf,0,sizeof(recvbuf));
    MsgHeader *RecvMsg = (MsgHeader*)recvbuf;
    char input[256];

    while(1)
    {   
        memset(input,0,sizeof(input));
        printf("Cli>");
        scanf("%[^\n]",input);
        fflush(stdin);
        Syntax_Cmd s_cmd = FTP_error;

        s_cmd = decode_in(input);
        if(s_cmd == FTP_error){
            printf("zsh: command not found:%s\n",input);
            continue;
        }else if(s_cmd == FTP_quit){
            printf("ftp>Bye!\n");
            break;
        }

        memset(&SendMsg, 0 ,sizeof(MsgHeader));
        SendMsg.last = false;

        char *buf = (char*)calloc(CACHE_SIZE, sizeof(char));
        if(s_cmd == FTP_get){
            SendMsg.s_cmd = s_cmd;
            SendMsg.DorI = INFO;
            SendMsg.last = true;
            SendMsg.error = false;
            memcpy(SendMsg.data, input+OtableLen[s_cmd], strlen(input)-OtableLen[s_cmd]);
            char *sendbuf =(char*) &SendMsg;
            send(sockCli,sendbuf, sizeof(SendMsg)+1, 0);
            RecvMsg->last = false;


            Readbolck recvb;
            memset(&recvb,0,sizeof(recvb));
            strncpy(recvb.filepath, Clientpath,strlen(Clientpath));
            strcat(recvb.filepath, input + OtableLen[s_cmd]);
            
            while (RecvMsg->last==false)
            {
                recv(sockCli, recvbuf, sizeof(SendMsg)+1, 0);    //接收来自服务器的数据
                printf("recv:%d\n", RecvMsg->cur_size);
                recvb.cur_size = RecvMsg->cur_size;
                recvb.cache = RecvMsg->data;
                recvb.method = BY_ASCII;
                put_in_file(&recvb,CACHE_SIZE-100);
                send(sockCli,sendbuf, sizeof(SendMsg)+1, 0);
            }
            
        }
            
    }
    close(sockCli);       //关闭套接字
    return 0;
}