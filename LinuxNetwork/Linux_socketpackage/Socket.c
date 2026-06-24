#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "Socket.h"

// 创建监听的套接字
int createSocket()
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		perror("socket");
		return -1;
	}
	printf("Socket created successfully, fd = %d\n", fd);
	return fd;
}

// 绑定本地的 IP 和端口 + 设置监听
int setListen(int lfd, unsigned short port)
{
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = INADDR_ANY;

	int ret = bind(lfd, (struct sockaddr*)&saddr, sizeof(saddr));
	if (ret == -1)
	{
		perror("bind");
		return -1;
	}
	printf("Socket bound successfully, ret = %d\n", ret);

	ret = listen(lfd, 128);
	if (ret == -1)
	{
		perror("listen");
		return -1;
	}
	
	printf("Listening started successfully, ret = %d\n", ret);
	
	return ret;
}

// 阻塞并等待客户端的连接
int acceptConn(int lfd, struct sockaddr_in* addr)
{
	int cfd = -1;
	if (addr == NULL)
	{
		cfd = accept(lfd, NULL, NULL);
	}
	else
	{
		// addr 为传出参数，将客户端的 IP 和端口信息传出
		int addrlen = sizeof(struct sockaddr_in);
		cfd = accept(lfd, (struct sockaddr*)addr, &addrlen);
	}
	if (cfd == -1)
	{
		perror("accept");
		return -1;
	}

	printf("Successfully connected with the client...\n");

	return cfd;
}

// 接收指定长度的字符串
int readn(int fd, char* buf, int size)
{
	char* pt = buf;// 指向存储数据的内存
	int count = size;// 记录剩余待接收的字节数

	while (count > 0)
	{
		int len = recv(fd, pt, count, 0);
		if (len == -1)
		{
			return -1;
		}
		else if (len == 0)
		{
			return size - count;
		}

		pt += len;
		count -= len;
	}

	return size;
}


// 接收数据
int recvMsg(int cfd, char** msg)
{
	// msg 为传出参数，保存接收的数据
	//接收报头
	int len = 0;
	readn(cfd, (char*)&len, 4);
	len = ntohl(len);

	printf("Size of the data block to be received: %d\n", len);

	char* data = (char*)malloc(len + 1);// + 1 为 '\0'
	int length = readn(cfd, data, len);
	if (length != len)
	{
		printf("Failed to receive data...\n");
		close(cfd);
		free(data);
		return -1;
	}

	data[len] = '\0';
	*msg = data;

	return length;
}

// 发送指定长度的字符串
int writen(int fd, const char* msg, int size)
{
	const char* buf = msg;// 指向发送数据的起始地址
	int count = size;// 记录剩余待发送的字节数

	while (count > 0)
	{
		int len = send(fd, buf, count, 0);
		if (len == -1)
		{
			return -1;
		}
		else if (len == 0)
		{
			continue;
		}

		buf += len;
		count = count - size;
	}

	return size;
}

// 发送数据
int sendMsg(int cfd, const char* msg, int len)
{
	// 异常判断
	if (cfd < 0 || msg == NULL || len <= 0)
	{
		return -1;
	}

	// 报头 + 数据
	char* data = (char*)malloc(len + 4);
	int biglen = htonl(len);
	memcpy(data, &biglen, 4);
	memcpy(data + 4, msg, len);

	int ret = writen(cfd, data, len + 4);
	if (ret == -1)
	{
		close(cfd);
	}

	return ret;
}

// 连接服务器
int connectToHost(int fd, const char* ip, unsigned short port)
{
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET, ip, &saddr.sin_addr.s_addr);

	int ret = connect(fd, (struct sockaddr*)&saddr, sizeof(saddr));
	if (ret == -1)
	{
		perror("connect");
		return -1;
	}

	printf("Successfully connected to the server...\n");

	return ret;
}

// 关闭套接字
int closeSocket(int fd)
{
	int ret = close(fd);
	if (ret == -1)
	{
		perror("close");
	}

	return ret;
}