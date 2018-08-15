#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "commit.h"

#define SIZE 1024*20

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

int main()
{
    char buf[1024*10] = {0};
    if (GetQueryString(buf) < 0)
    {
        fprintf(stderr, "GetQueryString failed\n");
        return 1;
    }
    char* data[2] = {0};
    Splite(buf, data);
    char Email[100] = {0};
    char pass[100] = {0};

    sscanf(data[0], "Email=%s", Email);
    sscanf(data[1], "pass=%s", pass);
    // 建立句柄
    MYSQL* connect_fd = mysql_init(NULL);
    // 建立连接
    if (mysql_real_connect(connect_fd, "127.0.0.1", "root", "", "shanlv", 3306, NULL, 0) == NULL)
    {
        fprintf(stderr, "mysql_readl_connect failed\n");
        return 1;
    }
    fprintf(stderr, "mysql_readl_connect OK\n");

    char sql[200] = {0};
    sprintf(sql, "select nickname from shan where md5=md5(\"%s%s\")", Email, pass);
    int ret = mysql_query(connect_fd, sql);
    if (ret < 0)
    {
        fprintf(stderr, "mysql_query failed\n");
        return 1;
    }

    // 遍历结果集合
    // 拿到result就可以通过result来进行获取
    MYSQL_RES* result = mysql_store_result(connect_fd);
    if (result == NULL)
    {
        fprintf(stderr, "select failed!");
        return 1;
    }

    /* printf("<html>"); */
    // 得到有几行几列
    int rows = mysql_num_rows(result);
    int fields = mysql_num_fields(result);

    // 这是获取到一个表头
    MYSQL_FIELD* f = mysql_fetch_field(result);
    while (f != NULL)
    {
        // f->name 表示这列的具体列名字
        /* printf("%s:", f->name); */
        f = mysql_fetch_field(result);
    }
    char name[128] = {0};
    int i = 0;
    for (; i < rows; ++i)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        int j = 0;
        for (; j < fields; ++j)
        {
            sprintf(name, "%s", row[j]);
            fprintf(stderr, "who login name == %s\n", name);
        }
    }
    if (strlen(name) == 0)
    {
        // 登陆失败
        fprintf(stderr, "登陆失败");
        FILE* fd = fopen("./root/404/login_failed.html", "r");
        if (fd == 0)
        {   
            fprintf(stderr, "open error\n");
            return 1;
        }   
        char c = '\0';
        struct stat sta;
        int st = stat("./root/404/login_failed.html", &sta);
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

        return 1;
    }
    int head = open("./root/myDB/select_login_head.html", O_RDONLY);
    if (head < 0)
    {
        fprintf(stderr, "select open select_login_head.html failed! line 96");
        return 1;
    }
    int mid = open("./root/myDB/select_login_mid.html", O_RDONLY);
    if (mid < 0)
    {
        fprintf(stderr, "select open select_login_mid.html failed! line 102");
        return 1;
    }
    char *tmp_buf = (char*)malloc(sizeof(char)*SIZE);
    // 先读首部
    int rd = read(head, tmp_buf, SIZE);
    if (rd < 0)
    {
        fprintf(stderr, "read head 113\n");
    }
    printf("%s", tmp_buf);
    /* write(0, tmp_buf, strlen(tmp_buf)); */
    // 打印出中间部分
    printf("<li class=\"sigi\"><a data-toggle=\"modal\" data-target=\"#myModal4\" >:%s</a></li>", name);
    // 清空
    memset(tmp_buf, 0, sizeof(tmp_buf));
    // 读取底部
    rd = read(mid, tmp_buf, SIZE);
    if (rd < 0)
    {
        fprintf(stderr, "read mid 124\n");
    }
    printf("%s", tmp_buf);
    /* write(0, tmp_buf, strlen(tmp_buf)); */

    free(tmp_buf);

    mysql_close(connect_fd);
    return 0;
}

