#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
    // 1. 创建用于通信的套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(1);
    }

    // 2. 连接服务器
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;     // ipv4
    addr.sin_port = htons(10000);   // 服务器监听的端口, 字节序应该是网络字节序
    inet_pton(AF_INET, "192.168.248.131", &addr.sin_addr.s_addr);

    int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect");
        exit(1);
    }

    // 通信
    int i = 0;
    char buf[1024] = { 0 };
    while (i < 200)
    {
        sprintf(buf, "hello world: %d\n...", i++);
        printf("%s\n", buf);

        //fgets(recvBuf, sizeof(recvBuf), stdin);
        write(fd, buf, strlen(buf) + 1);

        recv(fd, buf, sizeof(buf), 0);

        printf("recv msg: %s\n", buf);

        usleep(100000);
    }

    // 关闭文件描述符
    close(fd);

    return 0;
}