/* #pragma once */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int GetQueryString(char output[])
{
    // 先从环境变量中获取方法
    char* method = getenv("REQUEST_METHOD");
    if (method == NULL)
    {
        fprintf(stderr, "REQUEST_METHOD");
        return -1;
    }

    // 获取方法为GET，拿取QUERY_STRING
    if (strcasecmp(method, "GET") == 0)
    {
        char* query_string = getenv("QUERY_STRING");
        if (query_string == NULL)
        {
            fprintf(stderr, "QUERY_STRING");
            return -1;
        }
        strcpy(output, query_string);
    }
    else
    {
        // 是POST方法 获取CONTENT_LENGTH
        // 再从标准输入中读
        char* content_length_str = getenv("CONTENT_LENGTH");
        if (content_length_str == NULL)
        {
            fprintf(stderr, "CONTENT_LENGTH");
            return -1;
        }
        int content_length = atoi(content_length_str);
        int i = 0;
        for (; i < content_length; ++i)
        {
            read(0, &output[i], 1);
        }
        output[i] = '\0';
    }
    return 0;
}

int main()
{
    // 获取对应的参数
    // 如果是get方法就在query_string 中获取，
    // 如果是post方法就在body中获取
    // a&b
    char buf[1024*10] = {0};
    int ret = GetQueryString(buf);
    if (ret < 0)
    {
        fprintf(stderr, "GetQueryString");
        return 1;
    }
    int a, b;
    sscanf(buf, "a=%d&b=%d", &a, &b);
    // 进行业务计算
    printf("<html><h1>c = %d</h1></html>",a+b);
    return 0;
}
