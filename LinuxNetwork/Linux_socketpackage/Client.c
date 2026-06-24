#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "Socket.h"

int main()
{
	// 创建通信的套接字
	int fd = createSocket();
	if (fd == -1)
	{
		return -1;
	}

	// 连接服务器 IP 与 Port
	int ret = connectToHost(fd, "192.168.248.131", 10000);
	if (ret == -1)
	{
		return -1;
	}

	// 通信
	int fd1 = open("english.txt", O_RDONLY);
	int length = 0;
	char tmp[1000];
	while ((length = read(fd1, tmp, rand() % 1000)) > 0)
	{
		sendMsg(fd, tmp, length);

		memset(tmp, 0, sizeof(tmp));

		usleep(300);
	}

	sleep(10);

	// 关闭文件描述符
	closeSocket(fd);

	return 0;
}