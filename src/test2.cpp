#include<stdio.h>
#include <string.h>
#include <dirent.h>

#define  FilePath "ServerFile"

int main(){
	int i = 0;
	int filesize = 0;  
     	DIR *dir = NULL;  
        struct dirent *entry;  
      
        if((dir = opendir(FilePath))==NULL)  
        {  
      		printf("opendir failed!");  
      		return -1;  
        }
	else  
        {  
     		 while(entry=readdir(dir))  
      		{  
      			i++;
       			printf("filename%d = %s\n",i,entry->d_name);  //����ļ�����Ŀ¼������
//      			printf("filetype = %d\n",entry->d_type);  //����ļ�����   
     		}  
      
     		 closedir(dir);    
     }  
	return 0;
}
