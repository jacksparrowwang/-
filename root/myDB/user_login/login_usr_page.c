#include <stdio.h>
#include <fcntl.h>
#include "commit.h"
#define SIZE 1024

int main()
{
    char buf[128] = {0};
    if (GetQueryString(buf) < 0)
    {
        fprintf(stderr, "GetQueryString error\n");
        return 1;
    }


    int head = open("./root/myDB/select_login_head.html", O_RDONLY);
    if (head < 0)
    {
        fprintf(stderr, "open error 17\n");
        return 1;
    }
    char file_path[128] = {0};
    sprintf(file_path, "./root/%s", buf);
    FILE* tail = fopen(file_path, "r");
    if (tail == 0)
    {
        fprintf(stderr, "open error 25\n");
        return 1;
    }

    char cookie_buf[512] = {0};
    char* cookie = getenv("HTTP_COOKIE");
    sprintf(cookie_buf, "<li class=\"sigi\"><a data-toggle=\"modal\" data-target=\"#myModal4\" >:%s</a></li>", cookie);
    char* file = (char*)malloc(sizeof(char)*SIZE*20);
    read(head, file, SIZE*20);
    printf("%s", file);
    printf("%s", cookie_buf);
    fseek(tail, strlen(file)+strlen(cookie_buf)+
          strlen("data-toggle=\"modal\" \
                 data-target=\"#myModal4\">11111</fcntl>"), SEEK_SET);
    fread(file, SIZE*20, 1, tail);
    printf("%s", file);
    close(head);
    fclose(tail);
    return 0;
}
