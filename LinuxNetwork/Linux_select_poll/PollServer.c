#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <poll.h>

int main()
{
    // 1.创建套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket");
        exit(0);
    }

    // 2. 绑定 ip, port
    struct sockaddr_in addr;
    addr.sin_port = htons(9999);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    int ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind");
        exit(0);
    }

    // 3. 监听
    ret = listen(lfd, 100);
    if (ret == -1)
    {
        perror("listen");
        exit(0);
    }

    // 4. 等待连接 -> 循环
    // 检测 -> 读缓冲区, 委托内核去处理
    // 数据初始化, 创建自定义的文件描述符集
    struct pollfd fds[1024];// 创建一个 pollfd 类型的数组，用来保存要交给 poll() 检测的文件描述符

    // 初始化
    for (int i = 0; i < 1024; ++i)
    {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }
    fds[0].fd = lfd;

    int maxfd = 0;
    while (1)
    {
        // 委托内核检测
        ret = poll(fds, maxfd + 1, -1);
        if (ret == -1)
        {
            perror("select");
            exit(0);
        }

        // 检测的度缓冲区有变化
        // 有新连接
        if (fds[0].revents & POLLIN)// 判断 fds[0] 对应的 socket 是否可以读，即是否有新客户端连接来了
        {
            // 接收连接请求
            struct sockaddr_in sockcli;
            int len = sizeof(sockcli);

            // 这个 accept 是不会阻塞的
            int connfd = accept(lfd, (struct sockaddr*)&sockcli, &len);

            // 委托内核检测 connfd 的读缓冲区
            int i;
            for (i = 0; i < 1024; ++i)
            {
                if (fds[i].fd == -1)
                {
                    fds[i].fd = connfd;
                    break;
                }
            }

            maxfd = i > maxfd ? i : maxfd;
        }

        // 通信, 有客户端发送数据过来
        for (int i = 1; i <= maxfd; ++i)
        {
            // 如果在集合中, 说明读缓冲区有数据
            if (fds[i].revents & POLLIN)// 判断 fds[0] 对应的 socket 是否可以读，即是否有客户端发送数据过来了
            {
                char buf[128];
                int ret = read(fds[i].fd, buf, sizeof(buf));
                if (ret == -1)
                {
                    perror("read");
                    exit(0);
                }
                else if (ret == 0)
                {
                    printf("client disconnected...\n");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
                else
                {
                    printf("client say: %s\n", buf);
                    write(fds[i].fd, buf, strlen(buf) + 1);
                }
            }
        }
    }

    close(lfd);
    return 0;
}
