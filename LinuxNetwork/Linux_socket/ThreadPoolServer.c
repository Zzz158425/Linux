#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<pthread.h>
#include "ThreadPool.h"

// 信息结构体
struct SockInfo
{
	struct sockaddr_in addr;// ip 地址
	int fd;// 文件描述符
};
typedef struct PoolInfo
{
	ThreadPool* p;
	int fd;
}PoolInfo;

// 通信任务
void working(void* arg)
{
	struct SockInfo* pinfo = (struct SockInfo*)arg;

	// 打印客户端的地址信息
	char ip[24] = { 0 };
	printf("client IP address: %s, Port: %d\n",
		inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, ip, sizeof(ip)),
		ntohs(pinfo->addr.sin_port));

// 5.通信
	while (1)
	{
		// 接收数据
		char buff[1024];
		int len = recv(pinfo->fd, buff, sizeof(buff), 0);
		if (len > 0)
		{
			printf("client say: %s\n", buff);
			send(pinfo->fd, buff, len, 0);
		}
		else if (len == 0)
		{
			printf("Client disconnected...\n");
			break;
		}
		else
		{
			perror("recv");
			break;
		}
	}

	// 关闭文件描述符
	close(pinfo->fd);
	pinfo->fd = -1;
}

// 建立连接任务
void acceptConn(void* arg)
{
	PoolInfo* poolInfo = (PoolInfo*)arg;
// 4. 阻塞等待并接受客户端连接并用子线程来进行通信
	int addrlen = sizeof(struct sockaddr_in);
	while (1)
	{
		struct SockInfo* pinfo;
		pinfo = (struct SockInfo*)malloc(sizeof(struct SockInfo));

		// 阻塞等待并接受客户端连接
		pinfo->fd = accept(poolInfo->fd, (struct sockaddr*)&pinfo->addr, &addrlen);// 存储客户端地址等信息
		if (pinfo->fd == -1)
		{
			perror("accept");
			return;
		}

		//添加通信的任务
		threadPoolAdd(poolInfo->p, working, pinfo);
	}

	close(poolInfo->fd);
}

int main()
{
// 1.创建通信的套接字
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		perror("socket");
		return -1;
	}

// 2.绑定本地的 IP port
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(10000);
	saddr.sin_addr.s_addr = INADDR_ANY;  // 这个宏的值为 0 == 0.0.0.0

	int ret = bind(fd, (struct sockaddr*)&saddr, sizeof(saddr));
	if (ret == -1)
	{
		perror("bind");
		return -1;
	}

// 3.设置监听
	ret = listen(fd, 128);
	if (ret == -1)
	{
		perror("listen");
		return -1;
	}

	// 创建线程池并初始化结构体数组
	ThreadPool* pool = threadPoolCreate(3, 8, 100);

	PoolInfo* info = (PoolInfo*)malloc(sizeof(PoolInfo));
	info->p = pool;
	info->fd = fd;

	threadPoolAdd(pool, acceptConn, info);

	pthread_exit(NULL);

	return 0;
}




