#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "commit.h"
#define MAX 1024

void Splite(char first_line[], char **output)
{
    // 这里为了线程安全问题
    char* tmp = NULL;
    char* token = strtok_r(first_line, "&", &tmp);
    int i = 0;
    while (token != NULL)
    {   
        output[i] = token;
        ++i;
        token = strtok_r(NULL, "&", &tmp);                                                                               
    }   
}

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
    // 获取数据
    char buf[1024*10] = {0};
    if (GetQueryString(buf) < 0)
    {
        fprintf(stderr, "GetQueryString failed\n");
        return 1;
    }
    char nickname[50] = {0};
    char Email[50] = {0};
    char pass[50] = {0};
    char md5[50] = {0};
    char* data[3] = {0};
    Splite(buf, data);
    sscanf(data[0], "nickname=%s", nickname);
    sscanf(data[1], "Email=%s", Email);
    sscanf(data[2], "pass=%s", pass);
    
    decode(nickname);
    int i = 0;
    for (; i < (int)strlen(Email); ++i)
    {
        md5[i] = Email[i];
    }
    int j = 0;
    for (; j < (int)strlen(pass); ++j)
    {
        md5[i] = pass[j];
        ++i;
    }

    MYSQL* connect_fd = mysql_init(NULL);
    if (mysql_real_connect(connect_fd, "127.0.0.1", "root", "", "shanlv", 3306, NULL, 0) == NULL)
    {
        fprintf(stderr, "mysql_real_connect failed\n");
        return 1;
    }
    char sql[1024] = {0};
    // 已经修改
    sprintf(sql, "insert into shan values ('%s', '%s', '%s', md5('%s'))", nickname, Email, pass, md5);
    int ret = mysql_query(connect_fd,sql);
    if (ret < 0)
    {
        fprintf(stderr, "mysql_query failed\n");
        return 1;
    }
    fprintf(stderr, "insert success\n");
    mysql_close(connect_fd);
    FILE* fd = fopen("./root/404/success.html", "r");
    if (fd == 0)
    {
        fprintf(stderr, "open error\n");
        return 1;
    }
    char c = '\0';
    struct stat sta;
    int st = stat("./root/404/success.html", &sta);
    if (st < 0)
    {
        fprintf(stderr, "stat error\n");
        return 1;
    }
    while (fread(&c, 1, sta.st_size, fd) > 0)
    {
        fwrite(&c, 1, sta.st_size, stdout);
    }
    fclose(fd);
    return 0;
}

