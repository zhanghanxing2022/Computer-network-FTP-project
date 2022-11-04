#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include"header\defines.h"
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
 
    char sendbuf[sizeof(struct MsgHeader)+1];     //申请一个发送数据缓存区
    char recvbuf[sizeof(struct MsgHeader)+1];     //申请一个接收数据缓存区
    memset(sendbuf,0,sizeof(sendbuf));
    memset(recvbuf,0,sizeof(recvbuf));
    while(1)
    {
        recv(sockCli, recvbuf, 256, 0);    //接收来自服务器的数据
        printf("Ser:>\n");
        printf("%s\n",recvbuf);
        printf("Cli:>");
        scanf("%[^\n]",sendbuf);
        //清空缓冲区
        fflush(stdin);
        if(strncmp(sendbuf,"quit", 4) == 0)    //如果客户端发送的数据为"quit"，则退出。
            break;
        //put
        if(strncmp(sendbuf,"put ", 4) == 0){
            //Control Connection
            struct MsgHeader ControlMsg;
            ControlMsg.MsgType=Control;
            ControlMsg.Cmdtype=FTP_put;
            ControlMsg.data_size=strlen(sendbuf);
            ControlMsg.last=true;
            memcpy(ControlMsg.data,sendbuf,strlen(sendbuf));
            send(sockCli, (char*)&ControlMsg, sizeof(struct MsgHeader)+1, 0);
            //printf("after send control connection\n");
            /*
            待完善：Control Connection Response
            struct MsgHeader* recv_msg;
            do{
                send(sockCli, (char*)&ControlMsg, sizeof(struct MsgHeader)+1, 0);
                printf("send control connection\n");
                recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);
                recv_msg=(MsgHeader*)recvbuf;
                printf("receive control connection\n");
            }while(recv_msg->last==0);*/
    
            struct MsgHeader DataMsg;
            DataMsg.MsgType=Data;
            DataMsg.Cmdtype=FTP_put;
            DataMsg.data_size=0;
            //只有1个报文
            DataMsg.last=1;

            //read from file and store in block
            struct Readbolck block;
            memset(&block,0,sizeof(block));
            char *buf = (char*)calloc(CACHE_SIZE, sizeof(char));
            block.cache=buf;
            strncpy(block.filepath,"ClientFile/",11);
            strncpy(block.filepath+11, sendbuf + 4, CACHE_SIZE-4);
            //printf("filepath:%s\n",block.filepath);
            //memcpy(block.filepath, filename, CACHE_SIZE*sizeof(char));
            while(block.error==false&&block.lst == false)
            {
                //printf("in reading cycle\n");
                read_from_file(&block, CACHE_SIZE);
            }
            DataMsg.data_size=block.cur_size;
            strcpy(DataMsg.data,block.cache);
            //printf("%s\n",DataMsg.data);
            send(sockCli, (char*)&DataMsg, sizeof(struct MsgHeader)+1, 0);
            continue;
        }
        if(strncmp(sendbuf,"mkdir ", 6) == 0){
            struct MsgHeader ControlMsg;
            ControlMsg.MsgType=Control;
            ControlMsg.Cmdtype=FTP_mkdir;
            ControlMsg.data_size=strlen(sendbuf);
            ControlMsg.last=true;
            memcpy(ControlMsg.data,sendbuf,strlen(sendbuf));
            send(sockCli, (char*)&ControlMsg, sizeof(struct MsgHeader)+1, 0);
            continue;
        }
        else {
            struct MsgHeader msg;
            send(sockCli, sendbuf, strlen(sendbuf)+1, 0);   //发送数据
        }
    }
    close(sockCli);       //关闭套接字
    return 0;
}