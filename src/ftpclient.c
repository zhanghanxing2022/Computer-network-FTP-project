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
void printrecv(char* sendbuf, char* recvbuf, int sockCli);

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
                if(Control_Connection(sendbuf, recvbuf, sockCli)==-1){
                    printf("Control Connection failed\n");
                    break;
                }
                printf("Bye!\n");
                sleep(1);
                exit(0);
                //close(sockCli);
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
    memset(&ControlMsg,0,sizeof(ControlMsg));
    ControlMsg.MsgType=Control;
    ControlMsg.s_cmd=decode_in(sendbuf);
    ControlMsg.data_size=strlen(sendbuf)-OtableLen[ControlMsg.s_cmd];
    ControlMsg.last=true;
    ControlMsg.error=false;
    memcpy(ControlMsg.data,sendbuf+OtableLen[ControlMsg.s_cmd],strlen(sendbuf)-OtableLen[ControlMsg.s_cmd]);
    send(sockCli, (char*)&ControlMsg, sizeof(struct MsgHeader)+1, 0);
    
    //recevice
    struct MsgHeader* recv_msg;
    recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);
    recv_msg=(MsgHeader*)recvbuf;
    if(recv_msg->error) printf("%s\n",recv_msg->data);
    else if(recv_msg->s_cmd==FTP_delete||recv_msg->s_cmd==FTP_mkdir||recv_msg->s_cmd==FTP_cd||recv_msg->s_cmd==FTP_quit)printf("Done\n");
    if(recv_msg->last==1) return 0;//connection success
    else return -1;
}

int Client_get(char* sendbuf, char* recvbuf, int sockCli){
    if(Control_Connection(sendbuf, recvbuf, sockCli)==-1){
        printf("Control Connection failed\n");
        return -1;
    }
    struct MsgHeader SendMsg;
    SendMsg.s_cmd=FTP_get;
    SendMsg.MsgType=Data;
    struct MsgHeader *RecvMsg = (struct MsgHeader*)recvbuf;
    RecvMsg->last = false;
    Readbolck recvb;
    memset(&recvb,0,sizeof(recvb));
    strncpy(recvb.filepath, Clientpath,strlen(Clientpath));
    strcat(recvb.filepath, sendbuf + 4);
    remove(recvb.filepath);
    while (RecvMsg->last==false)
    {
        do{
            recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);    //接收来自服务器的数据
            memset(&SendMsg,0,sizeof(SendMsg));
            SendMsg.error=RecvMsg->error;
            send(sockCli, (char*)&SendMsg, sizeof(struct MsgHeader)+1, 0);
        }while(RecvMsg->error==true);
        
        printf("Cli>recv:%d\n", RecvMsg->data_size);
        recvb.cur_size = RecvMsg->data_size;
        recvb.cache = RecvMsg->data;
        printf("%s\n",RecvMsg->data);
        recvb.method = BY_ASCII;
        put_in_file(&recvb,recvb.cur_size);
        //send(sockCli, (char*)&SendMsg, sizeof(struct MsgHeader)+1, 0);
    }
}

int Client_put(char* sendbuf, char* recvbuf, int sockCli){
    //read from file and store in block
    struct Readbolck block;
    memset(&block,0,sizeof(block));
    char *buf = (char*)calloc(CACHE_SIZE, sizeof(char));
    block.cache=buf;
    strncpy(block.filepath, Clientpath,strlen(Clientpath));
    strcat(block.filepath, sendbuf + 4);

    //is the file exists?
    block.descriptor=file_type(block.filepath);
    if(block.descriptor == NOT_FOUND){
        block.error = true;
        printf("No such file\n");
        return -1;
    }

    if(Control_Connection(sendbuf, recvbuf, sockCli)==-1){
        printf("Control Connection failed\n");
        return -1;
    }

    //Data Connection
    struct MsgHeader DataMsg;
    memset(&DataMsg,0,sizeof(DataMsg));
    DataMsg.MsgType=Data;
    DataMsg.s_cmd=FTP_put;
    DataMsg.data_size=0;
    DataMsg.last=0;


    //recevice
    struct MsgHeader* recv_msg;
    recv_msg=(MsgHeader*)recvbuf;
    while(block.error==false&&block.lst == false)
    {
        read_from_file(&block, CACHE_SIZE);
        DataMsg.data_size=block.cur_size;
        strcpy(DataMsg.data,block.cache);        
        DataMsg.last=block.lst;
        do{
            send(sockCli, (char*)&DataMsg, sizeof(struct MsgHeader)+1, 0);
            recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);
        }while(recv_msg->error==true);
    }
    if(recv_msg->last==1) return 0;
    else return -1;
}

void printrecv(char* sendbuf, char* recvbuf, int sockCli){
    struct MsgHeader* recv_msg;
    recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);
    recv_msg=(MsgHeader*)recvbuf;
    printf("%s\n",recv_msg->data);
}
