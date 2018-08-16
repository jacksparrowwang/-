#include "http_server.h"

void InitFirstLine(FirstLine* first)
{
    first->method = NULL;
    first->query_string = NULL;
    first->url_path = NULL;
    first->version = NULL;
}


void InitHeader(Header* header)
{
    header->content_length = 0;
    header->content_type = NULL;
    header->host = NULL;
    header->user_agent = NULL;
}

int GetLine(char first_line[], int sock)
{
    char c = '\0';
    int i = 0;
    for (; i < SIZE; ++i)
    {
        // 读取首行，每次读一个
        ssize_t rv = recv(sock, &c, 1, 0);
        if (rv < 0)
        {
            perror("recv error\n");
            return -1;
        }
        if (rv == 0)
        {
            return -2;
        }
        first_line[i] = c;
        if (c == '\r')
        {
            // 这里是窥探下一个字符, 放到c中
            if (recv(sock, &c, 1, MSG_PEEK) < 0)
            {
                perror("header error\n");
                return -1;
            }
            if (c == '\n')
            {
                // 这里是窥探为'\n',则进行取'\n'
                recv(sock, &c, 1, 0);
                // 如果读到的是\r\n 我们就换成\n
                first_line[i] = c;
                break;
            }
            else
            {
                // 下一个不为\n, 说明浏览器就是用\r换行
                first_line[i] = '\n';
                break;
            }
        }
        if (c == '\n')
        {
            break;
        }
    }
    first_line[i+1] = '\0';
    return 0;
}

long long FileSize(char* file_path)
{
    struct stat st;
    int ret = stat(file_path, &st);
    if (ret < 0)
    {
        perror("stat error 76\n");
        return -1;
    }
    return st.st_size;
}

// 判断是否为目录文件
int Is_Dir(const char* file_path)
{
    struct stat st;
    int ret = stat(file_path, &st);
    if (ret < 0)
    {
        perror("stat error 88 http_server.c");
        return 0;
    }
    if (S_ISDIR(st.st_mode))
    {
        return 1;
    }
    return 0;
}

void FilePath(char* url_path, char* file_path)
{
    sprintf(file_path, "./root%s", url_path);
    if (file_path[strlen(file_path) - 1] == '/')
    {
        strcat(file_path, "index.html");
    }
    else if (Is_Dir(file_path))
    {
        strcat(file_path, "/index.html");
    }
}

int StaticPage(int sock, FirstLine* first)
{
    char file_path[SIZE] = {0};
    FilePath(first->url_path, file_path);
    long long size = FileSize(file_path);
    if (size == -1)
    {
        Error404(sock);
        return -1;
    }
    int fd = open(file_path, O_RDONLY);
    if (fd < 0)
    {
        Error404(sock);
        perror("open error");
        return -1;
    }
    char response[SIZE] = {0};
    sprintf(response, "HTTP/1.1 200 OK\nConten-Length: %llu\n\n", size);
    send(sock, response, strlen(response), 0);
    sendfile(sock, fd, NULL, size);
    close(fd);
    close(sock);
    return 0;
}

int FatherProcess(int sock, FirstLine* first, Header* header, int father_read, int father_write)
{
    if (strcasecmp(first->method, "POST") == 0)
    {
        char c = '\0';
        int i = 0;
        for (; i < header->content_length; ++i)
        {
            read(sock, &c, 1);
            write(father_write, &c, 1);
        }
    }
    // 在管道中，当所有的写端关闭，这时候就会读到EOF
    // 所以要进行fork后要进行关闭没用的文件描述符
    wait(NULL);
    char *buffer = (char*)malloc(sizeof(char) * SIZE * 30);
    size_t count = 0;
    // 从子进程中读取响应的body
    char c = '\0';
    while (read(father_read, &c, 1) > 0)
    {
        buffer[count] = c;
        ++count;
    }
    char response[SIZE] = {0};
    // 构造响应
    sprintf(response, "HTTP/1.1 200 OK\nContent-Length: %lu\n\n", count);
    send(sock, response, strlen(response), 0);
    // 这里是不能sendfile
    send(sock, buffer, count, 0);
    // 进程等待
    free(buffer);
    return 0;
}

int ChildProcess(FirstLine* first, Header* header, int child_read, int child_write)
{
    // 设置环境变量
    char method_env[SIZE] = {0};
    sprintf(method_env, "REQUEST_METHOD=%s", first->method);
    putenv(method_env);
    if (strcasecmp(first->method, "GET") == 0)
    {
        // QUERY_STRING
        char query_string_env[SIZE] = {0};
        sprintf(query_string_env, "QUERY_STRING=%s", first->query_string);
        putenv(query_string_env);
    }
    else
    {
        // CONTENT_LENGTH
        char content_length_env[SIZE] = {0};
        sprintf(content_length_env, "CONTENT_LENGTH=%llu", header->content_length);
        putenv(content_length_env);
    }
    // 这里是设置cookie 有字段就设置进去，没有就设置NULL
    char cookie[SIZE] = {0};
    sprintf(cookie, "HTTP_COOKIE=%s", header->cookie);
    putenv(cookie);
    // 重定向标准输入和标准输出
    dup2(child_read, 0);
    dup2(child_write, 1);
    // 进程替换
    char file_path[SIZE] = {0};
    // 拼接路径
    FilePath(first->url_path, file_path);
    
    // 传NULL，是为了命令行参数结束
    execl(file_path, file_path, NULL);
    // 不要让子进程去干扰父进程的的执行
    exit(0);
}

int DynamicsPage(int sock, FirstLine* first, Header* header)
{
    int fd1[2], fd2[2];
    int ret = pipe(fd1);
    if (ret < 0)
    {
        perror("pip1 error\n");
        return -1;
    }
    ret = pipe(fd2);
    if (ret < 0)
    {
        perror("pip2 error\n");
        return -1;
    }
    int father_read = fd1[0];
    int child_write = fd1[1];
    int father_write = fd2[1];
    int child_read = fd2[0];

    pid_t pid = fork();
    if (pid > 0)
    {
        // father
        // 如果是 POST请求，把body部分的数据读出来写道管道中
        // 构造HTTP响应首行 header 空行
        // 从管道中读取数据
        close(child_read);
        close(child_write);
        FatherProcess(sock, first, header, father_read, father_write);
        close(father_read);
        close(father_write);
    }
    else if (pid == 0)
    {
        // child
        // 设置环境变量(REQUEST_METHOD QUERY_STRING CONTENT_LENGHT)
        // 把标准输入输重定向到管道上
        // 子进程进行程序替换
        close(father_read);
        close(father_write);
        ChildProcess(first, header, child_read, child_write);
        close(child_read);
        close(child_write);
    }
    else
    {
        perror("fork");
    }
    return 0;
}

void Response(int sock, FirstLine* first, Header* header)
{
    // 构造响应
    // 静态页面
    // 如果是GET方法，并且没有query_string说明访问的是静态页面
    if (strcasecmp(first->method, "GET") == 0 && first->query_string == NULL)
    {
        int ret = StaticPage(sock, first);
        if (ret < 0)
        {
            Error404(sock);
        }
    }
    else if (strcasecmp(first->method, "POST") == 0 && header->content_length == 0)
    {
        // error
        Error404(sock);
    }
    else if (strcasecmp(first->method, "GET") == 0 && first->query_string != NULL)
    {
        // 动态页面
        // 如果是get方法且有query_string是动态页面
        // post方法，没有body标识错误 ，有body是动态页面
        int ret = DynamicsPage(sock, first, header);
        if (ret < 0)
        {
            Error404(sock);
        }
    }
    else if (strcasecmp(first->method, "POST") == 0)
    {
        // 动态页面
        // 如果是get方法且有query_string是动态页面
        // post方法，没有body标识错误 ，有body是动态页面
        int ret = DynamicsPage(sock, first, header);
        if (ret < 0)
        {
            Error404(sock);
        }
    }
    else
    {
        Error404(sock);
    }
}

void Error404(int sock)
{
    char buf[SIZE] = {0};
    char buf1[SIZE*10] = {0};
    int fd = open("./root/404/404.html", O_RDONLY);
    if (fd < 0)
    {
        perror("open 404");
        return;
    }
    struct stat st;
    int sta = stat("./root/404/404.html", &st);
    if (sta < 0)
    {
        perror("stat error318");
        return;
    }
    sprintf(buf, "HTTP/1.1 404 Not Found\nContent-Length: %lu\n\n", st.st_size);
    send(sock, buf, strlen(buf), 0);
    read(fd, buf1, st.st_size);
    send(sock, buf1, st.st_size, 0);
    close(fd);
    close(sock);
}

void Splite(char first_line[], char **output)
{
    // 这里为了线程安全问题
    char* tmp = NULL;
    char* token = strtok_r(first_line, " ", &tmp);
    int i = 0;
    while (token != NULL)
    {
        output[i] = token;
        ++i;
        token = strtok_r(NULL, " ", &tmp);
    }
}

void GetUrlPathQuer(char* url, FirstLine* first)
{
    int i = 0;
    for (; i < (int)strlen(url); ++i)
    {
        if (*(url+i) == '?')
        {
            break;
        }
    }
    if (i == (int)strlen(url))
    {
        first->url_path = url;
        first->query_string = NULL;
    }
    else
    {
        first->url_path = url;
        *(url+i) = '\0';
        // 是？后面开始+1
        first->query_string = url+i+1;
    }
}

int GetMethodUrlVer(FirstLine* first, char first_line[])
{
    char* buf[5] = {0};
    Splite(first_line, buf);
    first->method = buf[0];
    first->version = buf[2];
    GetUrlPathQuer(buf[1], first);
    return 0;
}

// 处理header
int ParseHeader(FirstLine* first, Header* header)
{
    // 此处用hash是比较好的
    if (strcasecmp(first->method, "POST") == 0)
    {
        char* content_length = strstr(header->header, "Content-Length: ");
        if (content_length != NULL)
        {
            content_length += strlen("Content-Length: ");
            char* cur = content_length;
            while (*cur != '\n')
            {
                ++cur;
            }
            *cur = '\0';
            header->content_length = atoi(content_length);
            *cur = '\n';
        }
    }
    char* cookie = strstr(header->header, "Cookie: ");
    if (cookie != NULL)
    {
        cookie += strlen("Cookie: ");
        char* cur = cookie;
        while (*cur != '\n')
        {
            ++cur;
        }
        *cur = '\0';
        header->cookie = cookie;
    }
    return 0;
}

void Requeset(int sock) // start
{
    FirstLine first;
    InitFirstLine(&first);
    char first_line[SIZE] = {0};
    int ret = GetLine(first_line, sock);
    if (ret < 0)
    {
        if (ret == -2)
        {
            return;
        }
        // 这里要注意是404页面处理
        Error404(sock);
        perror("GetFirstLine error\n");
        return;
    }

    ret = GetMethodUrlVer(&first, first_line);
    if (ret < 0)
    {
        // 404页面错误处理
        Error404(sock);
        perror("GetMethodUrlVer error\n");
        return;
    }

    Header header;
    InitHeader(&header);
    int total = 0;
    while (1)
    {
        char buf[SIZE] = {0};
        GetLine(buf, sock);
        if (strcmp(buf, "\n") == 0)
        {
            break;
        }
        strcpy(header.header + total, buf);
        total += strlen(buf);
    }

    // 处理header
    ret = ParseHeader(&first, &header);
    if (ret < 0)
    {
        // 错误处理
        Error404(sock);
        perror("GetWithHeader error\n");
        return;
    }

    Response(sock, &first, &header);
    close(sock);
}// end

void *ThreadEntry(void *arg)
{
    long sock = (long)arg;
    Requeset(sock);
    return NULL;
}

int InitSock(char* ip, int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket error");
        return -1;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);
    int b = bind(sock, (struct sockaddr*)&server, sizeof(server));
    if (b < 0)
    {
        perror("bind error\n");
        return -1;
    }

    int l = listen(sock, 5);
    if (l < 0)
    {
        perror("listen error");
        return -1;
    }
    return sock;
}


int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIG_IGN);
    if (argc != 3)
    {
        perror("Usage:./server Ip port");
        return 1;
    }

    printf("server start OK!\n");
    int listen_sock = InitSock(argv[1], atoi(argv[2]));
    if (listen_sock < 0)
    {
        perror("listen_sock errror\n");
        return 2;
    }

    while (1)
    {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        long new_sock = accept(listen_sock, (struct sockaddr*)&client, &len);
        if (new_sock < 0)
        {
            perror("accept error\n");
            continue;
        }

        pthread_t thd;
        if (pthread_create(&thd, NULL, ThreadEntry, (void*)new_sock) != 0)
        {
            perror("pthread_create error\n");
            continue;
        }
        pthread_detach(thd);
    }

    return 0;
}
