#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
void get_all(int argc, char** argv){
    if(argc == 1)
        perror(argv[0]);
    struct dirent *dp;
    DIR *dfd;
    
}