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
char current_path[256];

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
    // if(res == -1)
    //     perror("connect");
    // else
    //     printf("Client Connect Server OK.\n");
    while (res == -1)
    {
        perror("connect");
        res = connect(sockCli, (struct sockaddr *)&addrSer, addrlen);
    }
    printf("Client Connect Server OK.\n");
 
    char sendbuf[sizeof(struct MsgHeader)+1];     //申请一个发送数据缓存区
    char recvbuf[sizeof(struct MsgHeader)+1];     //申请一个接收数据缓存区
    memset(sendbuf,0,sizeof(sendbuf));
    memset(recvbuf,0,sizeof(recvbuf));
    // 初始化路径名
    memset(current_path, 0, sizeof(current_path));
    strcpy(current_path, "root");

    struct MsgHeader msg;
    while(1)
    {
        // 需要清空sendbuf
        memset(sendbuf, 0, sizeof(sendbuf));
        printf("%s:>", current_path);
        scanf("%[^\n]", sendbuf);

        //清空缓冲区
        fflush(stdin);
        Syntax_Cmd s_cmd = FTP_error;

        int site[16][2] = {0};
        int param_num = check_param(sendbuf, site, 256);
        if (param_num == 0)
            continue;
        char* command = malloc(sizeof(char) * (site[0][1] - site[0][0] + 1));
        memset(command,0,sizeof(command));
        strncpy(command, &sendbuf[site[0][0]], site[0][1] - site[0][0] + 1);
        s_cmd = check_command(command);

        // 检测指令是否正确
        if (s_cmd == FTP_error)
        {
            printf("zsh: command not found\n");
            continue;
        }
        else if (s_cmd == FTP_quit)
        {
            printf("%s:>Bye!\n", current_path);
            break;
        }
        // 判断参数个数是否正确,比如 "cd file_a b"返回3，是违法指令
        if (param_num != ParamNum[s_cmd])
        {
            printf("wrong command!\n");
            continue;
        }

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

int Control_Connection(char *sendbuf, char *recvbuf, int sockCli)
{
    int site[16][2] = {0};
    int param_num = check_param(sendbuf, site, 256);
    char command[site[0][1] - site[0][0] + 1];
    strncpy(command, &sendbuf[site[0][0]], site[0][1] - site[0][0] + 1);
    Syntax_Cmd s_cmd = check_command(command);
    // send control connection
    struct MsgHeader ControlMsg;
    memset(&ControlMsg, 0, sizeof(ControlMsg));
    ControlMsg.MsgType = Control;
    ControlMsg.s_cmd = s_cmd;
    ControlMsg.data_size = strlen(sendbuf) - OtableLen[ControlMsg.s_cmd];
    ControlMsg.last = true;
    ControlMsg.error = false;
    // 因为ls后面没有参数
    if (param_num == 2)memcpy(ControlMsg.data, &sendbuf[site[1][0]], site[1][1] - site[1][0] + 1);
    // memcpy(ControlMsg.data, sendbuf + OtableLen[ControlMsg.s_cmd], strlen(sendbuf) - OtableLen[ControlMsg.s_cmd]);
    send(sockCli, (char *)&ControlMsg, sizeof(struct MsgHeader) + 1, 0);

    // recevice
    struct MsgHeader *recv_msg;
    recv(sockCli, recvbuf, sizeof(struct MsgHeader) + 1, 0);
    recv_msg = (MsgHeader *)recvbuf;
    if (recv_msg->error)
        printf("%s\n", recv_msg->data);
    else if (recv_msg->s_cmd == FTP_delete || recv_msg->s_cmd == FTP_mkdir || recv_msg->s_cmd == FTP_cd || recv_msg->s_cmd == FTP_quit)
    {
        if (recv_msg->s_cmd == FTP_cd)
        {
            memset(current_path, 0, sizeof(current_path));
            strcpy(current_path, recv_msg->data);
        }
        // printf("Done\n");
    }
    if (recv_msg->last == 1)
        return 0; // connection success
    else
        return -1;
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
    int i;
    for(i=4; i<strlen(sendbuf);i++){
        if(sendbuf[i]=='\\'){
            sendbuf[i] = '/';
        }
    }
    char *p = strrchr(sendbuf+4,'/');
    p = p==NULL?sendbuf + 3:p;
    strcat(recvb.filepath, p+1);
    remove(recvb.filepath);//如果存在，则删除文件
    while (RecvMsg->last==false)
    {   
        recv(sockCli, recvbuf, sizeof(struct MsgHeader)+1, 0);    //接收来自服务器的数据
        if(RecvMsg->error == true ){
            printf("%s\n",RecvMsg->data);
            return 1;
        }
        
        recvb.cur_size = RecvMsg->data_size;
        recvb.cache = RecvMsg->data;
        recvb.method = BY_ASCII;
        put_in_file(&recvb,recvb.cur_size);
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
