#pragma once
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <fcntl.h>

#define SIZE 1024

typedef struct FirstLine
{
    char* method;
    char* url_path;
    char* query_string;
    char* version;
}FirstLine;

typedef struct Header
{
    char header[SIZE * 4];
    long long content_length;
    char* content_type;
    char* host;
    char* user_agent;
    char* cookie;
}Header;

// 子进程处理
int ChildProcess(FirstLine* first, Header* header, int child_read, int child_write);

// 动态页面父进程处理
int FatherProcess(int sock, FirstLine* first, Header* header, int father_read, int father_write);

// 处理静态页面
int StaticPage(int sock, FirstLine* first);

// 处理动态页面
int DynamicsPage(int sock, FirstLine* first, Header* header);

// 解析头部
int ParseHeader(FirstLine* first, Header* header);

// 处理错误
void Error404(int sock);

// 拼接出文件或者可执行程序的路径
void FilePath(char* url_path, char* file_path);

// 计算出文件的大小
long long FileSize(char* file_path);

// 获取一行
int GetLine(char first_line[], int sock);

// 从url中获取url_path method version
int GetMethodUrlVer(FirstLine* first, char first_line[]);

// 获取url query_string
void GetUrlPathQuer(char* url, FirstLine* first);

// 初始化首行
void InitFirstLine(FirstLine* first);

// 初始化header
void InitHeader(Header* header);

// 初始化
int InitSock(char* ip, int port);

// 判断是否为目录
int Is_Dir(const char* file_path);

// 请求
void Requeset(int sock);

// 响应
void Response(int sock, FirstLine* first, Header* header);

// 字符串切割
void Splite(char first_line[], char **output);

// 线程入口函数
void* ThreadEntry(void *arg);
