#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
int main(){
	char path[256] = "root/123";
	char p[256] = "serverfile";
    char *ptr = strchr(path, '/');
    int len = ptr - path;
	printf("%d\n",strncmp(path,"root",len)); 
	strncat(p,ptr,sizeof(path) -len);
	printf("%s\n",p);
}
