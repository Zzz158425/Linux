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

// 信息结构体
typedef struct socketinfo
{
	int fd;
	int epfd;
}SocketInfo;

// 连接线程函数
void* acceptConn(void* arg)
{
	printf("acceptConn tid:%ld\n", pthread_self());

	SocketInfo* info = (SocketInfo*)arg;

	int cfd = accept(info->fd, NULL, NULL);

	// 设置非阻塞属性
	int flag = fcntl(cfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(cfd, F_SETFL, flag);

	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = cfd;

	int ret = epoll_ctl(info->epfd, EPOLL_CTL_ADD, cfd, &ev);
	if (ret == -1)
	{
		perror("epoll_ctl");
		exit(1);
	}

	free(info);

	return NULL;
}

// 通信线程函数
void* communication(void* arg)
{
	printf("communication tid:%ld\n", pthread_self());

	SocketInfo* info = (SocketInfo*)arg;

	char buf[5];
	char temp[1024];
	bzero(temp, sizeof(temp));

	while (1)
	{
		int len = recv(info->fd, buf, sizeof(buf), 0);
		if (len == -1)
		{
			if (errno == EAGAIN)
			{
				printf("Data reception completed...\n");
				send(info->fd, temp, strlen(temp) + 1, 0);
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
			epoll_ctl(info->epfd, EPOLL_CTL_DEL, info->fd, NULL);
			close(info->fd);
			break;
		}

		// 小写转大写
		for (int i = 0; i < len; i++)
		{
			buf[i] = toupper(buf[i]);
		}

		write(STDOUT_FILENO, buf, len);
		//printf("after buf = %s\n", buf);

		strncat(temp, buf, len);
	}

	free(info);

	return NULL;
}

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

		pthread_t tid;

		for (int i = 0; i < num; ++i)
		{
			// 接收数据
			int fd = evs[i].data.fd;

			// 初始化结构体
			SocketInfo* info = (SocketInfo*)malloc(sizeof(SocketInfo));
			info->fd = fd;
			info->epfd = epfd;

			if (fd == lfd)
			{
				pthread_create(&tid, NULL, acceptConn, info);
				pthread_detach(tid);
			}
			else
			{
				pthread_create(&tid, NULL, communication, info);
				pthread_detach(tid);
			}
		}
	}

	// 关闭文件描述符
	close(lfd);

	return 0;
}