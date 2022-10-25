#pragma once
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
enum FILE_MODE{
    DIR,
    FILE,
    NOT_FOUND
};

FILE_MODE file_type(const char* filename)
{
    struct stat statbuf;
    
    if (stat( filename, &statbuf) == -1)

        return NOT_FOUND;
    if (S_ISDIR(statbuf.st_mode))
        return DIR;
    
    if (S_ISREG(statbuf.st_mode))
        return FILE;

}