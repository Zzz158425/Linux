#include<stdio.h>
#include<ctype.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<errno.h>
#include<pthread.h>

int main()
{
	// 创建监听的套接字
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		perror("socket");
		exit(1);
	}

	// 初始化结构体
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(10000);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 设置端口复用
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// 绑定
	int ret = bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret == -1)
	{
		perror("bind");
		exit(1);
	}

	// 监听
	ret = listen(lfd, 128);
	if (ret == -1)
	{
		perror("listen");
		exit(1);
	}

	// 创建 epoll 实例
	int epfd = epoll_create(1);
	if (epfd == -1)
	{
		perror("epoll_create");
		exit(1);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = lfd;

	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
	if (ret == -1)
	{
		perror("epoll_ctl");
		exit(1);
	}

	struct epoll_event evs[1024];
	int size = sizeof(evs) / sizeof(evs[0]);

	while (1)
	{
		int num = epoll_wait(epfd, evs, size, -1);
		printf("num = %d\n", num);
		for (int i = 0; i < num; ++i)
		{
			// 接收数据
			int fd = evs[i].data.fd;
			if (fd == lfd)
			{
				int cfd = accept(fd, NULL, NULL);

				// 设置非阻塞属性，把客户端通信 socket cfd 设置成非阻塞模式
				// ET 边沿触发模式只通知一次，如果你没有一次性把数据读完，剩下的数据不会再次触发通知
				// 如果 socket 是阻塞的，你在 while (1) 里一直 recv()，当数据读完后，程序会卡死在 recv()，等新数据
				int flag = fcntl(cfd, F_GETFL);
				flag |= O_NONBLOCK;
				fcntl(cfd, F_SETFL, flag);

				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = cfd;

				ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);// 此时会将 cfd 拷贝到 epfd 中
				if (ret == -1)
				{
					perror("epoll_ctl");
					exit(1);
				}
			}
			else
			{
				char buf[5];

				while (1)
				{
					int len = recv(fd, buf, sizeof(buf), 0);
					if (len == -1)
					{
						if (errno == EAGAIN)// 当前 socket 接收缓冲区里已经没有数据可读了
						{
							printf("Data reception completed...\n");
							break;
						}
						else
						{
							perror("recv");
							exit(1);
						}
					}
					else if (len == 0)
					{
						printf("client disconnected...\n");
						epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
						close(fd);
						break;
					}

					write(STDOUT_FILENO, buf, len);// 将 buf 输出到当前终端，buf 已写满，无 \0，会使 printf 打印乱码内容 

					// 小写转大写
					for (int i = 0; i < len; i++)
					{
						buf[i] = toupper(buf[i]);
					}

					write(STDOUT_FILENO, buf, len);
					//printf("after buf = %s\n", buf);

					// 发送数据
					ret = send(fd, buf, len, 0);
					if (ret == -1)
					{
						perror("send");
						return -1;
					}
				}
				
			}
		}
	}

	// 关闭文件描述符
	close(lfd);

	return 0;
}