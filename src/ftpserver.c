#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#elif defined __APPLE__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <sys/types.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "dirent.h"
#include "function.h"
#define SERVER_PORT 5050
#define SERVER_IP "127.0.0.1"
#define QUEUE_SIZE 5

#pragma pack(1) // 设置结构体1字节对齐**************

struct MsgHeader
{
    /* data */
};
#pragma pack()

int main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        printf("Error: Unable to start WSAS\n");
    }
#endif
    int sockSer;
    sockSer = socket(AF_INET, SOCK_STREAM, 0);
    if (sockSer == -1)
    {
        perror("socket");
    }
#ifdef _WIN32
    char yes[256] = "1";
#elif defined __APPLE__
    int temp = 1;
    int *yes = &temp;
#endif

    if (setsockopt(sockSer, SOL_SOCKET, SO_REUSEADDR, yes, sizeof(int)) == -1)
    {
        perror("socket");
    }

    struct sockaddr_in addrSer, addrCli;            //创建一个记录地址信息的结构体
    addrSer.sin_family = AF_INET;                   //创建一个记录地址信息的结构体
    addrSer.sin_port = htons(SERVER_PORT);          //设置地址结构体中的端口号
    addrSer.sin_addr.s_addr = inet_addr(SERVER_IP); //设置其中的服务器ip

    socklen_t addrlen = sizeof(struct sockaddr);
    int res = bind(sockSer, (struct sockaddr *)&addrSer, addrlen);
    if (res == -1)
    {
        perror("bind");
    }

    listen(sockSer, QUEUE_SIZE);

    printf("Server Wait Client Accept………\n ");

    int sockConn = accept(sockSer, (struct sockaddr *)&addrCli, &addrlen);
    if (sockConn == -1)
        perror("accept");
    else
    {
        printf("Server Accept Client OK.\n");
        printf("Client IP:> %s\n", inet_ntoa(addrCli.sin_addr));
        printf("Client Port:> %d\n", ntohs(addrCli.sin_port));
    }

    char sendbuf[256]; //申请一个发送数据缓存区
    char recvbuf[256]; //申请一个接收数据缓存区
    memset(sendbuf, 0, sizeof(sendbuf));
    memset(recvbuf, 0, sizeof(recvbuf));
    // 当前路径
    char current_path[256] = "ServerFile";
    // 用戶當前路徑
    char client_current_path[256] = "root";
    while (1)
    {
        memset(sendbuf, 0, sizeof(sendbuf));
        recv(sockConn, recvbuf, 256, 0);
        // printf("Cli:> %s\n", recvbuf);
        // printf("Ser:>");
        // scanf("%[^\n]",sendbuf);
        // fflush(stdin);
        // if(strncmp(sendbuf, "quit", 4) ==0)
        //     break;

        // 解析
        int site[16][2];
        int length = split(recvbuf, site);
        if (length == 0)
        {
            strcpy(sendbuf, "no command");
        }
        else
        {
            char command[256];
            memset(command, 0, sizeof(command));
            strncpy(command, &recvbuf[site[0][0]], site[0][1] - site[0][0] + 1);
            if (strncmp(command, "cd", 2) == 0)
            {
                if (length != 2)
                {
                    strcpy(sendbuf, "Illegal command");
                }
                else
                {
                    char filename[256];
                    memset(filename, 0, sizeof(filename));
                    strncpy(filename, &recvbuf[site[1][0]], site[1][1] - site[1][0] + 1);
                    int filesize = 0;
                    DIR *dir = NULL;
                    struct dirent *entry;
                    if ((dir = opendir(current_path)) == NULL)
                    {
                        strcpy(sendbuf, "file does not exit");
                    }
                    else
                    {
                        int flag = 0;
                        while (entry = readdir(dir))
                        {
                            if (strcmp(filename, entry->d_name) == 0)
                            {
                                flag = 1;
                                if (strcmp(filename, "..") == 0)
                                {
                                    // 找到最後一個'/',然後把它去掉
                                    char copy_path[128];
                                    memset(copy_path, 0, sizeof(copy_path));
                                    strcpy(copy_path, current_path);
                                    char *ptr = strrchr(copy_path, '/');
                                    if (ptr == NULL)
                                    {
                                        memset(sendbuf, 0, sizeof(sendbuf));
                                        strcpy(sendbuf, client_current_path);
                                        break;
                                    }
                                    int len = ptr - copy_path;
                                    memset(current_path, 0, sizeof(current_path));
                                    strncpy(current_path, copy_path, len);

                                    memset(copy_path, 0, sizeof(copy_path));
                                    strcpy(copy_path, client_current_path);
                                    ptr = strrchr(copy_path, '/');
                                    len = ptr - copy_path;
                                    memset(client_current_path, 0, sizeof(client_current_path));
                                    strncpy(client_current_path, copy_path, len);
                                }
                                else if (strcmp(filename, ".") == 0)
                                {
                                    // do nothing
                                }
                                else
                                {
                                    strcat(current_path, "/");
                                    strcat(current_path, filename);
                                    strcat(client_current_path, "/");
                                    strcat(client_current_path, filename);
                                }
                                memset(sendbuf, 0, sizeof(sendbuf));
                                strcpy(sendbuf, client_current_path);
                                break;
                            }
                        }
                        // 文件夾不存在
                        if (!flag) {
                            memset(sendbuf, 0, sizeof(sendbuf));
                            strcpy(sendbuf, "error1");
                        }
                        closedir(dir);
                    }
                }
            }
            // if (strncmp(sendbuf,"delete", 6) == 0) {

            // }
            if (strncmp(command,"pwd", 3) == 0) {
                memset(sendbuf, 0, sizeof(sendbuf));
                strcpy(sendbuf, client_current_path);
            }
        }
        send(sockConn, sendbuf, strlen(sendbuf) + 1, 0);
    }
    close(sockSer);
    return 0;
}