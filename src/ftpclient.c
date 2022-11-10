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
const char Clientpath[] = "./ClientFile/";

int Control_Connection(char* sendbuf, char* recvbuf, int sockCli);
int Client_get(char* sendbuf, char* recvbuf, int sockCli);
int Client_put(char* sendbuf, char* recvbuf, int sockCli);
int Client_ls(char* sendbuf, char* recvbuf, int sockCli);
void printrecv(char* sendbuf, char* recvbuf, int sockCli);

//decode command type
int command_type(char* cmd){
    if(strncmp(cmd,"get ", 4) == 0) return 0;
    if(strncmp(cmd,"put ", 4) == 0) return 1;
    if(strncmp(cmd,"delete ", 7) == 0) return 2;
    if(strncmp(cmd,"ls", 2) == 0) return 3;
    if(strncmp(cmd,"cd ", 3) == 0) return 4;
    if(strncmp(cmd,"mkdir ", 6) == 0) return 5;
    if(strncmp(cmd,"pwd", 3) == 0) return 6;
    if(strncmp(cmd,"quit", 4) == 0) return 7;
    return -1;
}

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

    struct MsgHeader msg;
    while(1)
    {
        printf("Cli:>");
        scanf("%[^\n]",sendbuf);
        //清空缓冲区
        fflush(stdin);
        Syntax_Cmd s_cmd=FTP_error;
        s_cmd=decode_in(sendbuf);

        switch(s_cmd){
            //get
            case 0:
                Client_get(sendbuf, recvbuf, sockCli);
                break;
            //put
            case 1:
                Client_put(sendbuf, recvbuf, sockCli);
                break;
            //delete
            case 2:
                if(Control_Connection(sendbuf, recvbuf, sockCli)==-1)
                    printf("Control Connection failed\n");
                break;
            //ls
            case 3:
                if(Control_Connection(sendbuf, recvbuf, sockCli)==-1){
                    printf("Control Connection failed\n");
                    break;
                }
                printrecv(sendbuf, recvbuf, sockCli);
                break;
            //cd
            case 4:
                if(Control_Connection(sendbuf, recvbuf, sockCli)==-1)
                    printf("Control Connection failed\n");
                break;
            //mkdir
            case 5:
                if(Control_Connection(sendbuf, recvbuf, sockCli)==-1)
                    printf("Control Connection failed\n");
                break;
            //pwd
            case 6:
                if(Control_Connection(sendbuf, recvbuf, sockCli)==-1){
                    printf("Control Connection failed\n");
                    break;
                }
                printrecv(sendbuf, recvbuf, sockCli);
                break;
            //quit
            case 7:
                printf("Bye!\n");
                close(sockCli);
                return 0;
            default:
                //printf("Invalid command\n");
                send(sockCli, sendbuf, strlen(sendbuf)+1, 0);   //发送数据
        }
    }
    close(sockCli);       //关闭套接字
    return 0;
}

int Control_Connection(char* sendbuf, char* recvbuf, int sockCli){
    //send control connection
    struct MsgHeader ControlMsg;
    ControlMsg.MsgType=Control;
    ControlMsg.Cmdtype=decode_in(sendbuf);
    ControlMsg.data_size=strlen(sendbuf)-OtableLen[ControlMsg.Cmdtype];
    ControlMsg.last=true;
    memcpy(ControlMsg.data,sendbuf+OtableLen[ControlMsg.Cmdtype],strlen(sendbuf)-OtableLen[ControlMsg.Cmdtype]);
    send(sockCli, (char*)&ControlMsg, sizeof(struct MsgHeader)+1, 0);
    
    //recevice
    struct MsgHeader* recv_msg;
    recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);
    recv_msg=(MsgHeader*)recvbuf;
    if(recv_msg->last==1) return 0;//connection success
    else return -1;
}

int Client_get(char* sendbuf, char* recvbuf, int sockCli){
     if(Control_Connection(sendbuf, recvbuf, sockCli)==-1){
        printf("Control Connection failed\n");
        return -1;
    }

    struct MsgHeader *RecvMsg = (struct MsgHeader*)recvbuf;
    RecvMsg->last = false;
    Readbolck recvb;
    memset(&recvb,0,sizeof(recvb));
    strncpy(recvb.filepath, Clientpath,strlen(Clientpath));
    strcat(recvb.filepath, sendbuf + 4);
    while (RecvMsg->last==false)
    {
        recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);    //接收来自服务器的数据
        printf("Cli>recv:%d\n", RecvMsg->data_size);
        recvb.cur_size = RecvMsg->data_size;
        recvb.cache = RecvMsg->data;
        printf("%s\n",RecvMsg->data);
        recvb.method = BY_ASCII;
        put_in_file(&recvb,recvb.cur_size);
    }
}

int Client_put(char* sendbuf, char* recvbuf, int sockCli){
    if(Control_Connection(sendbuf, recvbuf, sockCli)==-1){
        printf("Control Connection failed\n");
        return -1;
    }

    //Data Connection
    struct MsgHeader DataMsg;
    DataMsg.MsgType=Data;
    DataMsg.Cmdtype=FTP_put;
    DataMsg.data_size=0;
    //只有1个报文
    DataMsg.last=0;

    //read from file and store in block
    struct Readbolck block;
    memset(&block,0,sizeof(block));
    char *buf = (char*)calloc(CACHE_SIZE, sizeof(char));
    block.cache=buf;
    strncpy(block.filepath,"ClientFile/",11);
    strncpy(block.filepath+11, sendbuf + 4, CACHE_SIZE-4);
    while(block.error==false&&block.lst == false)
    {
        read_from_file(&block, CACHE_SIZE);
        DataMsg.data_size=block.cur_size;
        strcpy(DataMsg.data,block.cache);        
        DataMsg.last=block.lst;
        send(sockCli, (char*)&DataMsg, sizeof(struct MsgHeader)+1, 0);
    }
    //recevice
    struct MsgHeader* recv_msg;
    recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);
    recv_msg=(MsgHeader*)recvbuf;
    if(recv_msg->last==1) return 0;
    else return -1;
}

int Client_ls(char* sendbuf, char* recvbuf, int sockCli){
    if(Control_Connection(sendbuf, recvbuf, sockCli)==-1){
        printf("Control Connection failed\n");
        return -1;
    }
    //recevice
    struct MsgHeader* recv_msg;
    recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);
    recv_msg=(MsgHeader*)recvbuf;
    printf("%s\n",recv_msg->data);
}

void printrecv(char* sendbuf, char* recvbuf, int sockCli){
    struct MsgHeader* recv_msg;
    recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);
    recv_msg=(MsgHeader*)recvbuf;
    printf("%s\n",recv_msg->data);
}
