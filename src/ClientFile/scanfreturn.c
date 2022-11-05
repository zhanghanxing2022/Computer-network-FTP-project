#include<stdio.h>
#include<string.h>
int main(){
    char buf[255];
    memset(buf,0,sizeof(buf));
    int len = 0;
    printf("input:\n");
    len = scanf("%[^\n]",buf);
    printf("len:%d,LEN:%d\n,string:%s\n",len,strlen(buf),buf);
}