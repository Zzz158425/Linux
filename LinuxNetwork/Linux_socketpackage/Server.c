#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include "Socket.h"

struct SockInfo
{
	int fd;
	struct sockaddr_in addr;
};
struct SockInfo infos[250];

void* working(void* arg)
{
	struct SockInfo* pinfo = (struct SockInfo*)arg;
	// 连接建立成功，打印客户端的 IP 和端口信息
	char ip[32];
	printf("Client IP: %s, port: %d\n",
		inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, ip, sizeof(ip)),
		ntohs(pinfo->addr.sin_port));

	// 通信
	while (1)
	{
		char* buf;
		int len = recvMsg(pinfo->fd, &buf);
		printf("recv, %d: ...\n", len);
		if (len > 0)
		{
			printf("%s\n\n\n\n", buf);
			free(buf);
		}
		else
		{
			break;
		}

		sleep(1);
	}

	pinfo->fd = -1;

	return NULL;
}

int main()
{
	// 创建通信的套接字
	int fd = createSocket();
	if (fd == -1)
	{
		return -1;
	}

	// 绑定本地的 IP 与 Port + 设置监听
	int ret = setListen(fd, 10000);
	if (ret == -1)
	{
		return -1;
	}

	int max = sizeof(infos) / sizeof(infos[0]);
	for (int i = 0; i < max; i++)
	{
		bzero(&infos[i], sizeof(infos[0]));
		infos[i].fd = -1;
	}

	// 阻塞并等待客户端的连接
	while (1)
	{
		struct SockInfo* pinfo;
		printf("max: %d\n", max);
		for (int i = 0; i < max; i++)
		{
			if (infos[i].fd == -1)
			{
				pinfo = &infos[i];
				break;
			}
		}

		pinfo->fd = acceptConn(fd, &pinfo->addr);

		// 创建子线程
		pthread_t tid;
		pthread_create(&tid, NULL, working, pinfo);
		pthread_detach(tid);
	}

	// 关闭文件描述符
	closeSocket(fd);

	return 0;
}