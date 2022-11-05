#include <stdio.h>
#include <string.h>
int split(char *str, int site[16][2],int length)
{   
    int start[256] = {0};
    int end[256] = {0};
    int start_p = 0;
    int end_p = 0;
    if (str[0] != ' ')
    {
        start[start_p++] = 0;
    }
    int i;
    for (i = 1; i < length; i++)
    {
        if (str[i] != ' ' && str[i - 1] == ' ')
        {
            start[start_p++] = i;
        }
        if ((str[i] == ' ' || str[i] == '\0') && str[i - 1] != ' ' && str[i - 1] != '\0')
        {
            end[end_p++] = i - 1;
        }
    }
    if (end_p)
    {
        for (i = 0; i < end_p; i++)
        {
            site[i][0] = start[i];
            site[i][1] = end[i];
        }
    }
    return end_p;
}