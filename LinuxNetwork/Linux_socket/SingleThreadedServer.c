#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>

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
	// INADDR_ANY 代表本机的所有 IP, 假设有三个网卡就有三个 IP 地址
	// INADDR_ANY 这个宏可以代表任意一个 IP 地址
	// INADDR_ANY 这个宏一般用于本地的绑定操作
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

// 4. 阻塞等待并接受客户端连接
	struct sockaddr_in caddr;
	int addrlen = sizeof(caddr);
	int cfd = accept(fd, (struct sockaddr*)&caddr, &addrlen);
	if (cfd == -1)
	{
		perror("accept");
		return -1;
	}

// 打印客户端的地址信息
	char ip[24] = { 0 };
	printf("client IP address: %s, Port: %d\n",
		inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ip, sizeof(ip)),
		ntohs(caddr.sin_port));

// 5.通信
	while (1)
	{
		// 接收数据
		char buff[1024];
		int len = recv(cfd, buff, sizeof(buff), 0);
		if (len > 0)
		{
			printf("client say: %s\n", buff);
			send(cfd, buff, len, 0);
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

// 6.关闭文件描述符
	close(fd);
	close(cfd);

	return 0;
}