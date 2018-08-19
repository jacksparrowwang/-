#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include "../commit.h"
#define MAX 1024

int hex2dec(char c) {  
    if ('0' <= c && c <= '9') {  
        return c - '0';  
    } else if ('a' <= c && c <= 'f') {  
        return c - 'a' + 10;  
    } else if ('A' <= c && c <= 'F') {  
        return c - 'A' + 10;  
    } else {  
        return -1;  
    }  
} 

char dec2hex(short int c)  
{  
    if (0 <= c && c <= 9) {  
        return c + '0';  
    } else if (10 <= c && c <= 15) {  
        return c + 'A' - 10;  
    } else {  
        return -1;  
    }  
} 

void decoding(char url[])  
{  
    int i = 0;  
    int len = strlen(url);  
    int res_len = 0;  
    char res[MAX];  
    for (i = 0; i < len; ++i) {  
        char c = url[i];  
        if (('0' <= c && c <= '9') ||  
            ('a' <= c && c <= 'z') ||  
            ('A' <= c && c <= 'Z') || c == '/' || c == '.') {  
            res[res_len++] = c;  
        } else {  
            int j = (short int)c;  
            if (j < 0)  
                j += 256;  
            int i1, i0;  
            i1 = j / 16;  
            i0 = j - i1 * 16;  
            res[res_len++] = '%';  
            res[res_len++] = dec2hex(i1);  
            res[res_len++] = dec2hex(i0);  
        }  
    }  
    res[res_len] = '\0';  
    strcpy(url, res);  
}  

void decode(char url[])  
{  
    int i = 0;  
    int len = strlen(url);  
    int res_len = 0;  
    char res[MAX];  
    for (i = 0; i < len; ++i) {  
        char c = url[i];  
        if (c != '%') {  
            res[res_len++] = c;  
        } else {   
            char c1 = url[++i];  
            char c0 = url[++i];  
            int num = 0;  
            num = hex2dec(c1) * 16 + hex2dec(c0);  
            res[res_len++] = num;  
        }  
    }  
    res[res_len] = '\0';  
    strcpy(url, res);  
} 

int main()
{
    char buf[1024*10] = {0};
    if (GetQueryString(buf) < 0)
    {
        fprintf(stderr, "GetQueryString failed\n");
        return 1;
    }
    
    char* cookie = getenv("HTTP_COOKIE");
    FILE* comment_fd = fopen("./root/myDB/comment/comment_page.html", "w");
    if (comment_fd == 0)
    {
        fprintf(stderr, "open comment_fd failed\n");
        return 1;
    }
    FILE* head = fopen("./root/myDB/comment/comment_head.html", "a+");
    if (comment_fd == 0)
    {
        fprintf(stderr, "open comment_fd failed\n");
        return 1;
    }
    char div_comment[1024*10] = {0};
    decode(buf+8);
    sprintf(div_comment, "<h3>%s</h3>\n<div>\n%s\n</div>\n---------------------------------\n<br>", cookie, buf+8);
    fwrite(div_comment, strlen(div_comment), 1, head);
    char c = '\0';
    fseek(head, 0, SEEK_SET);
    while (fread(&c, 1, 1, head) > 0)
    {
        fwrite(&c, 1, 1, comment_fd);
    }
    
    FILE* tail = fopen("./root/myDB/comment/comment_file.html", "r");
    if (tail == 0)
    {
        fprintf(stderr, "open comment_file failed\n");
        return 1;
    }

    c = '\0';
    while (fread(&c, 1, 1, tail) > 0)
    {
        fwrite(&c, 1, 1, comment_fd);
    }
    fclose(head);
    fclose(tail);
    fclose(comment_fd);
    int success_page = open("./root/404/comment_success.html", O_RDONLY);
    if (success_page < 0)
    {
        fprintf(stderr, "open success_page failed\n");
        return 1;
    }
    while (read(success_page, &c, 1) > 0)
    {
        write(1, &c, 1);
    }
    close(success_page);
    return 0;
}
