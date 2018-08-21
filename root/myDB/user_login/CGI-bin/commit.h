/* #pragma once */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX 1024

static int hex2dec(char c) {  
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

static char dec2hex(short int c)  
{  
    if (0 <= c && c <= 9) {  
        return c + '0';  
    } else if (10 <= c && c <= 15) {  
        return c + 'A' - 10;  
    } else {  
        return -1;  
    }  
} 

static void decoding(char url[])  
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

static void decode(char url[])  
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


static int GetQueryString(char output[])
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

