#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<pthread.h>

// 子线程任务函数
// 信息结构体
struct SockInfo
{
	struct sockaddr_in addr;// ip 地址
	int fd;// 文件描述符
};
struct SockInfo infos[512];

void* working(void* arg)
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

	return NULL;
}

// 主线程
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
	saddr.sin_addr.s_addr = INADDR_ANY;  // 这个宏的值为0 == 0.0.0.0

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
		exit(0);
	}

// 初始化结构体数组
	int max = sizeof(infos) / sizeof(infos[0]);
	for (int i = 0; i < max; i++)
	{
		bzero(&infos[i], sizeof(infos[i])); // 初始化结构体数组为 0
		infos[i].fd = -1;
	}

// 4. 阻塞等待并接受客户端连接并用子线程来进行通信
	int addrlen = sizeof(struct sockaddr_in);
	while (1)
	{
		// 找空闲位置
		struct SockInfo* pinfo;
		for (int i = 0; i < max; i++)
		{
			if (infos[i].fd == -1)
			{
				pinfo = &infos[i];
				break;
			}
		}

		// 阻塞等待并接受客户端连接
		int cfd = accept(fd, (struct sockaddr*)&pinfo->addr, &addrlen);
		pinfo->fd = cfd;
		if (cfd == -1)
		{
			perror("accept");
			return -1;
		}
		
		// 创建子线程
		pthread_t tid;
		pthread_create(&tid, NULL, working, pinfo);
		pthread_detach(tid);
	}

// 6.关闭文件描述符
	close(fd);

	return 0;
}
