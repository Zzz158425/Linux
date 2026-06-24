#include<stdio.h>
#include<ctype.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/epoll.h>

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
	// 让服务器端口在程序重启时可以更快重新绑定，不容易因为端口还处于占用状态而 bind 失败
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
	ev.events = EPOLLIN;// 边沿触发：ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = lfd;

	// 把监听 socket lfd 添加到 epoll 实例 epfd 中，让 epoll 帮你监听 lfd 上的事件
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
	if (ret == -1)
	{
		perror("epoll_ctl");
		exit(1);
	}

	//struct epoll_event 
	//{
	//	uint32_t events;
	//	epoll_data_t data;
	//};
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

				ev.events = EPOLLIN;// 边沿触发：ev.events = EPOLLIN | EPOLLET;
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
				char buf[1024];
				int len = recv(fd, buf, sizeof(buf), 0);
				if (len == -1)
				{
					perror("recv");
					exit(1);
				}
				else if (len == 0)
				{
					printf("client disconnected...\n");
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
					close(fd);
					break;
				}

				printf("read buf = %s\n", buf);

				// 小写转大写
				for (int i = 0; i < len; i++)
				{
					buf[i] = toupper(buf[i]);
				}
				printf("after buf = %s\n", buf);

				// 发送数据
				ret = send(fd, buf, strlen(buf) + 1, 0);
				if (ret == -1)
				{
					perror("send");
					return -1;
				}
			}
		}
	}

	// 关闭文件描述符
	close(lfd);

	return 0;
}