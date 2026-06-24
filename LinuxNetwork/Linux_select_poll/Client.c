#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
	// 1. 创建用于通信的套接字
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		perror("socket");
		exit(0);
	}

	// 2. 连接服务器
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;// ipv4
	addr.sin_port = htons(10000);// 服务器监听的端口, 字节序应该是网络字节序
	// 发送的数据里，凡是要被对方当成多字节整数解释的字段，都应该转换成网络字节序
	// htons() 是整数的主机字节序转网络字节序；inet_pton() 是 IP 字符串转网络字节序的二进制地址
	inet_pton(AF_INET, "192.168.248.131", &addr.sin_addr.s_addr);// 主机字节序转网络字节序

	int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
	{
		perror("connect");
		exit(0);
	}

	// 通信
	int i = 0;
	while (1)
	{
		char recvBuf[1024];

		// 写数据
		sprintf(recvBuf, "data: %d\n", i++);// 把格式化后的字符串写入 recvBuf 数组中，recvBuf = "data: %d\n"
		//fgets(recvBuf, sizeof(recvBuf), stdin);
		write(fd, recvBuf, strlen(recvBuf) + 1);

		// 读数据
		// 如果服务器没有发送数据，read 默认阻塞
		int len = read(fd, recvBuf, sizeof(recvBuf));
		if (len == -1)
		{
			perror("read");
			exit(1);
		}

		printf("recv buf: %s\n", recvBuf);
		sleep(1);
	}

	// 释放资源
	close(fd);

	return 0;
}