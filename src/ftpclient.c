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
        scanf("%*[ |\t]%[^\n]",input);//*是跳过相应的字符项。表示跳过开头的空格或者制表符
        fflush(stdin);
        Syntax_Cmd s_cmd = FTP_error;

        s_cmd = decode_in(input);

        if(s_cmd == FTP_error){
            printf("zsh: command not found:%s\n",input);
            continue;
        }else if(s_cmd == FTP_quit){
            printf("Cli>Bye!\n");
            break;
        }

        memset(&SendMsg, 0 ,sizeof(MsgHeader));
        SendMsg.last = false;
        char *buf = (char*)calloc(CACHE_SIZE, sizeof(char));

        if(s_cmd == FTP_get){
            SendMsg.s_cmd = s_cmd;
            SendMsg.DorI = CONTROL;
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
                printf("Cli>recv:%d\n", RecvMsg->data_size);
                recvb.data_size = RecvMsg->data_size;
                recvb.cache = RecvMsg->data;
                recvb.method = BY_ASCII;
                put_in_file(&recvb,0);
                // send(sockCli,sendbuf, sizeof(SendMsg)+1, 0);
            }
            
        }else if(s_cmd==FTP_put){
            //Control Connection
            SendMsg.DorI=CONTROL;
            SendMsg.s_cmd=FTP_put;
            SendMsg.data_size=strlen(input)-OtableLen[s_cmd];
            SendMsg.last=false;
            memcpy(SendMsg.data,input+OtableLen[s_cmd],SendMsg.data_size);
            send(sockCli, (char*) &SendMsg, sizeof(struct MsgHeader)+1, 0);
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
            DataMsg.DorI=DATA;
            DataMsg.s_cmd=FTP_put;
            DataMsg.data_size=0;
            //只有1个报文
            DataMsg.last=1;

            //read from file and store in block
            struct Readbolck block;
            memset(&block,0,sizeof(block));
            char *buf = (char*)calloc(CACHE_SIZE, sizeof(char));
            block.cache=buf;
            block.method = BY_BIT;
            strncpy(block.filepath,Clientpath,sizeof(Clientpath));
            strcat(block.filepath, input + OtableLen[s_cmd]);
            printf("Cli>filepath:%s\n",block.filepath);
            //memcpy(block.filepath, filename, CACHE_SIZE*sizeof(char));
            while(block.error==false&&block.lst == false)
            {
                printf("Cli>in reading cycle\n");
                read_from_file(&block, CACHE_SIZE);
                DataMsg.data_size=block.data_size;
                DataMsg.last = block.lst;
                strcpy(DataMsg.data,block.cache);
                //printf("%s\n",DataMsg.data);
                send(sockCli, (char*)&DataMsg, sizeof(struct MsgHeader)+1, 0);
            }
            continue;
        }else if(s_cmd == FTP_mkdir){
            struct MsgHeader ControlMsg;
            memset(&ControlMsg,0,sizeof(ControlMsg));
            ControlMsg.DorI=CONTROL;
            ControlMsg.s_cmd=FTP_mkdir;
            ControlMsg.data_size=strlen(input)-OtableLen[s_cmd];
            ControlMsg.last=true;
            strncpy(ControlMsg.data,input + OtableLen[s_cmd],strlen(input)-OtableLen[s_cmd]);
            printf("Cli>input:%s,cut:%s,res:%s\n",input,input+OtableLen[s_cmd],ControlMsg.data);
            send(sockCli, (char*)&ControlMsg, sizeof(struct MsgHeader)+1, 0);
            continue;
        }
            
    }
    close(sockCli);       //关闭套接字
    return 0;
}