#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
int main(){
	char path[256] = "root/a/b/1.txt";
	char p[256] = "serverfile";
    char *ptr = strchr(path, '/');
    int len = ptr - path;
	printf("%d\n",strncmp(path,"root",len)); 
	strcat(p,ptr);
	printf("%s\n",p);
}
