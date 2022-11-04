#include "../header/file.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef char Buf[256];
void fetch_in(Readbolck*b){
    memcpy(b->filepath,"../ClientFile/hello",
    sizeof("../ClientFile/hello"));

}
void push_out(Readbolck*b){
    memcpy(b->filepath,"../ServerFile/hello",
    sizeof("../ServerFile/hello"));
}
int main(){
    char* buf =(char*)calloc(CACHE_SIZE, sizeof(char));
    Readbolck block;
    memset(&block,0,sizeof(block));
    block.cache=buf;
    block.method = BY_BIT;
    while (block.lst==false && block.error == false)
    {   
        fetch_in(&block);
        read_from_file(&block, 255);
        if(block.error == true){
            printf("not_found\n");
        }
        // for (int i = 0; i < block.cur_size; i++)
        // {
        //     printf("%c",block.cache[i]);
        // }
        push_out(&block);
        put_in_file(&block,block.cur_size);
        // printf("%d\n",block.cur_size);
    }

}