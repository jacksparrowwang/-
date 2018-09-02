#include <stdio.h>
#include <fcntl.h>
#include "commit.h"

int main()
{
    char buf[1024] = {0};
    if (GetQueryString(buf) < 0)
    {
        fprintf(stderr, "GetQueryString failed\n");
        return 1;
    }

    char file_path[1024] = {0};
    decode(buf);
    sprintf(file_path, "./root/data/%s", buf);
    printf("<html><head>\
           <title>%s</title>\
           <meta name=\"viewport\" content=\"width=device-width initial-scale=1\">\
           <meta http-equiv=\"Content-Type\" content=\"text/html charset=utf-8\">\
           </head><body><h2>%s</h2><div>", buf, buf+1);
    int minute_fd = open(file_path, O_RDONLY);
    if (minute_fd <  0)
    {
        fprintf(stderr, "open file_path failed\n");
        return 1;
    }
    char buf_tmp[1024*20] = {0};
    while (read(minute_fd, buf_tmp, sizeof(buf_tmp)) > 0)
    {
        printf("%s", buf_tmp);
    }
    printf("</div></body></html>");
    close(minute_fd);
    return 0;
}
