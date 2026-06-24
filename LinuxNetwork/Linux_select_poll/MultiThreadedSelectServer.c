#include<stdio.h>
#include<ctype.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<sys/select.h>

pthread_mutex_t mutex;// 互斥锁

// 接收客户端的连接线程
typedef struct fdinfo
{
	int fd;
	int *maxfd;
	fd_set* rdset;
}FDInfo;

void* acceptConn(void* arg)
{
	printf("thread ID: %ld\n", pthread_self());

	FDInfo* info = (FDInfo*)arg;

	// 接收客户端的连接
	int cfd = accept(info->fd, NULL, NULL);
	if (cfd == -1)
	{
		perror("accept");
		free(info);
		exit(1);
	}

	printf("new client connected, cfd = %d\n", cfd);

	pthread_mutex_lock(&mutex);

	FD_SET(cfd, info->rdset);
	*info->maxfd = cfd > *info->maxfd ? cfd : *info->maxfd;

	pthread_mutex_unlock(&mutex);

	free(info);

	return NULL;
}

// 通信线程
void* communication(void* arg)
{
	printf("thread ID: %ld\n", pthread_self());

	FDInfo* info = (FDInfo*)arg;

	// 接收数据
	char buf[1024];
	int len = recv(info->fd, buf, sizeof(buf), 0);
	if (len == -1)
	{
		perror("recv");
		free(info);
		exit(1);
	}
	else if (len == 0)
	{
		printf("client disconnected...\n");

		close(info->fd);

		free(info);

		return NULL;
	}

	printf("read buf = %s\n", buf);

	// 小写转大写
	for (int i = 0; i < len; i++)
	{
		buf[i] = toupper(buf[i]);
	}
	printf("after buf = %s\n", buf);

	// 发送数据
	int ret = send(info->fd, buf, strlen(buf) + 1, 0);
	if (ret == -1)
	{
		perror("send");

		free(info);

		exit(1);
	}

	// 处理完以后，把文件描述符 i 重新加入监听集合
	pthread_mutex_lock(&mutex);

	FD_SET(info->fd, info->rdset);

	pthread_mutex_unlock(&mutex);

	free(info);

	return NULL;
}

int main()
{
	pthread_mutex_init(&mutex, NULL);

	// 创建监听的套接字
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		perror("socket");
		exit(1);
	}

	// 绑定
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(10000);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

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

	// 创建读集合
	fd_set redset;
	FD_ZERO(&redset);
	FD_SET(lfd, &redset);

	int maxfd = lfd;

	while (1)
	{
		pthread_mutex_lock(&mutex);

		fd_set tmp = redset;

		pthread_mutex_unlock(&mutex);

		int ret = select(maxfd + 1, &tmp, NULL, NULL, NULL);
		if (ret == -1)
		{
			perror("select");
			exit(1);
		}

		// 如果 lfd 有事件，说明有新客户端连接
		if (FD_ISSET(lfd, &tmp))
		{
			// 创建子线程来处理
			pthread_t tid;
			FDInfo* info = (FDInfo*)malloc(sizeof(FDInfo));
			info->fd = lfd;
			info->maxfd = &maxfd;
			info->rdset = &redset;

			pthread_create(&tid, NULL, acceptConn, info);
			pthread_detach(tid);
		}

		// 不管有没有新连接，都要检查已有客户端有没有发数据
		for (int i = 0; i <= maxfd; i++)
		{
			if (i != lfd && FD_ISSET(i, &tmp))
			{
				// 创建子线程来处理
				pthread_t tid;
				FDInfo* info = (FDInfo*)malloc(sizeof(FDInfo));
				info->fd = i;
				info->maxfd = &maxfd;
				info->rdset = &redset;

				pthread_create(&tid, NULL, communication, info);
				pthread_detach(tid);
			}
		}
	}

	// 关闭监听的文件描述符
	close(lfd);

	pthread_mutex_destroy(&mutex);

	return 0;
}