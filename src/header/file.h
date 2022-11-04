#pragma once
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include<stdbool.h>
#define CACHE_SIZE 256
FILE *fp1,*fp2;
typedef enum FILE_MODE{
    A_DIR,
    A_FILE,
    NOT_FOUND
}FILE_MODE;
typedef enum CODE_METRIC{
    BY_ASCII,
    BY_BIT
}CODE_METRIC;

typedef struct Readbolck
{
    char filepath[256];
    bool lst;
    unsigned long tail;
    unsigned long long length;
    int cur_size;
    char *cache;
    FILE_MODE descriptor;
    CODE_METRIC method;
    bool error;
}Readbolck;

FILE_MODE file_type(const char* filename)
{
    struct stat statbuf;
    
    if (stat( filename, &statbuf) == -1)
        return NOT_FOUND;
    if (S_ISDIR(statbuf.st_mode))
        return A_DIR;
    if (S_ISREG(statbuf.st_mode))
        return A_FILE;
    return NOT_FOUND;
}
void read_from_file(Readbolck*block,int len){
    memset(block->cache,0,CACHE_SIZE);
    block->descriptor=file_type(block->filepath);
    if(block->descriptor == NOT_FOUND){
        block->error = true;
        printf("No such file\n");
        return;
    }
    fp1 = fopen(block->filepath,block->method==BY_ASCII?"r":"rb");
    if(fp1 == NULL){
        block->error = true;
        return;
    }
    fseek(fp1,block->tail,SEEK_SET);
    
    block->cur_size = fread(block->cache,sizeof(char),255,fp1);
    
    if(feof(fp1)!=0){
        block->lst = true;
    }
    block->tail = ftell(fp1);
    fclose(fp1);
}

void put_in_file(Readbolck*block,int len){
    if(block->error==true){
        return;
    }
    fp2 = fopen(block->filepath,block->method==BY_ASCII?"a+":"ab+");
    if(fp2 == NULL){
        block->error = true;
        return;
    }
    fwrite(block->cache, sizeof(char), len,fp2);
    fclose(fp2);
}