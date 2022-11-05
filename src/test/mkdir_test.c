#include <sys/types.h>
#include "../header/file.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
int main(){
    if(file_type("block") == NOT_FOUND){
        mkdir("block",0777);
    }
}