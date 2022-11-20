#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
int main(){
	char path[256] = "root/a/b/1.txt";
	char p[256] = "serverfile";
    char *ptr = strchr(path, '/');
    int len = ptr - path;
	printf("%d\n",(opendir("..\\src")) == NULL); 
	strcat(p,ptr);
	printf("%s\n",p);
	ptr = strchr(ptr + 1, '/');
	len = ptr - path;
	printf("%d\n",len);
}
