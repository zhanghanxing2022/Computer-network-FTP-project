#pragma once
#include <stdlib.h>
#include <string.h>
#include "file.h"
#define ORDER_SUM 8
typedef enum Syntax_Cmd
{
    FTP_get,
    FTP_put,
    FTP_delete,
    FTP_ls,
    FTP_cd,
    FTP_mkdir,
    FTP_pwd,
    FTP_quit,
    FTP_error
} Syntax_Cmd;
enum Msg_Data_Type
{
    Control,
    Data
};

#pragma pack(1) // 1字节对齐
typedef struct MsgHeader
{
    Syntax_Cmd s_cmd;
    enum Msg_Data_Type MsgType;
    int data_size;
    char data[CACHE_SIZE];
    bool last;
    bool error;
} MsgHeader;
#pragma pack()

static const char Otable[8][9] = {"get ", "put ", "delete ", "ls", "cd ", "mkdir ", "pwd", "quit"};
static const int OtableLen[8] = {4, 4, 7, 2, 3, 6, 3, 4};
static const Syntax_Cmd OtableMap[9] = {
    FTP_get,
    FTP_put,
    FTP_delete,
    FTP_ls,
    FTP_cd,
    FTP_mkdir,
    FTP_pwd,
    FTP_quit,
    FTP_error
};

static const int ParamNum[8] = {2, 2, 2, 1, 2, 2, 1, 1};

// 返回参数数量
int check_param(char *str, int site[16][2], int length)
{
    int start[256] = {0};
    int end[256] = {0};
    int start_p = 0;
    int end_p = 0;
    if (str[0] != ' ')
    {
        start[start_p++] = 0;
    }
    int i;
    for (i = 1; i < length; i++)
    {
        if (str[i] != ' ' && str[i - 1] == ' ')
        {
            start[start_p++] = i;
        }
        if ((str[i] == ' ' || str[i] == '\0') && str[i - 1] != ' ' && str[i - 1] != '\0')
        {
            end[end_p++] = i - 1;
        }
    }
    if (end_p)
    {
        for (i = 0; i < end_p; i++)
        {
            site[i][0] = start[i];
            site[i][1] = end[i];
        }
    }
    return end_p;
}
// 检查参数
Syntax_Cmd check_command(const char *argv)
{
    Syntax_Cmd res = FTP_error;
    int i;
    // 原本的定义有空格啥的我实在是搞不明白，把空格去掉了。但是上面的我没敢改，因为在别的地方用到了，
    // 但是这样会不会有一个问题，就是如果command后面多敲几个空格，最后读取到的参数实际上最前面是带空格的？
    // 还有就是不能只比较前n字节，否则如果输入cdc a就不会报错
    static const char Otable[8][9] = {"get", "put", "delete", "ls", "cd", "mkdir", "pwd", "quit"};
    static const int OtableLen[8] = {3, 3, 6, 2, 2, 5, 3, 4};
    for (i = 0; i < 8; i++)
    {
        if (strcmp(argv, Otable[i]) == 0)
        {
            res = OtableMap[i];
        }
    }
    return res;
}

int get_server_path(char *current_path, char *path, char *new_path, int size)
{
    strcpy(new_path, current_path);
    char _path[256];
    memset(_path, 0, sizeof(_path));
    strcpy(_path, path);
    strcat(_path, "/");
    char *ptr = strchr(_path, '/');
    char *last_ptr = _path;
    char temp[256];
    memset(temp, 0, sizeof(temp));
    int len = 0;
    while (ptr != NULL)
    {
        len = ptr - last_ptr;
        memset(temp, 0, sizeof(temp));
        strncpy(temp, last_ptr, len);
        //	对不同类型的temp处理
        if (strcmp(temp, "..") == 0)
        {
            char *p = strrchr(new_path, '/');
            if (p == NULL)
            {
                return 0;
            }
            else
            {
                int len = p - new_path;
                memset(p, 0, size - len);
            }
        }
        else if (strcmp(temp, ".") == 0)
        {
            // do nothing
        }
        else
        {
            if (strcmp(temp, "root") == 0 || strcmp(temp, "~") == 0)
            {
                memset(new_path, 0, sizeof(new_path));
                strcat(new_path, "ServerFile");
            }
            else
            {
                strcat(new_path, "/");
                strcat(new_path, temp);
            }
        }
        last_ptr = ptr + 1;
        ptr = strchr(ptr + 1, '/');
    }
    return 1;
}

void str_replace(char *str1, char *str2, char *str3)
{
    int i, j, k, done, count = 0, gap = 0;
    char temp[256];
    for (i = 0; i < strlen(str1); i += gap)
    {
        if (str1[i] == str2[0])
        {
            done = 0;
            for (j = i, k = 0; k < strlen(str2); j++, k++)
            {
                if (str1[j] != str2[k])
                {
                    done = 1;
                    gap = k;
                    break;
                }
            }
            if (done == 0)
            { // 已找到待替换字符串并替换
                for (j = i + strlen(str2), k = 0; j < strlen(str1); j++, k++)
                { // 保存原字符串中剩余的字符
                    temp[k] = str1[j];
                }
                temp[k] = '\0'; // 将字符数组变成字符串
                for (j = i, k = 0; k < strlen(str3); j++, k++)
                { // 字符串替换
                    str1[j] = str3[k];
                    count++;
                }
                for (k = 0; k < strlen(temp); j++, k++)
                { // 剩余字符串回接
                    str1[j] = temp[k];
                }
                str1[j] = '\0'; // 将字符数组变成字符串
                gap = strlen(str2);
            }
        }
        else
        {
            gap = 1;
        }
    }
    if (count == 0)
    {
        // printf("Can't find the replaced string!\n");
    }
    return;
}
void delete_more(char *str1, char *result)
{
    int i;
    int count = 0;
    for (i = 0; i < strlen(str1) - 1; i++)
    {
        if (str1[i] == '/' & str1[i + 1] != '/')
        {
            result[count] = str1[i];
            count++;
            continue;
        }
        if (str1[i] != '/')
        {
            result[count] = str1[i];
            count++;
            continue;
        }
    }
    result[count] = str1[strlen(str1) - 1];
}
void transform_path(char buf[256],char result[256]){
    str_replace(buf, "\\", "/");
    delete_more(buf, result);
}
