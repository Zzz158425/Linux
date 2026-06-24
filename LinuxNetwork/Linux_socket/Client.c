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

// 2.链接服务器的 IP port
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(10000);
	inet_pton(AF_INET, "192.168.248.131", &saddr.sin_addr.s_addr);

	int ret = connect(fd, (struct sockaddr*)&saddr, sizeof(saddr));
	if (ret == -1)
	{
		perror("connect");
		return -1;
	}

// 3.通信
	int number = 0;
	while (1)
	{
		// 发送数据
		char buff[1024];
		sprintf(buff, "helloworld!, %d ...\n", number++);
		send(fd, buff, strlen(buff) + 1, 0);

		// 接收数据
		memset(buff, 0, sizeof(buff));
		int len = recv(fd, buff, sizeof(buff), 0);
		if (len > 0)
		{
			printf("server say: %s\n", buff);
		}
		else if (len == 0)
		{
			printf("server disconnected...\n");
			break;
		}
		else
		{
			perror("recv");
			break;
		}
		sleep(1);
	}

// 6.关闭文件描述符
	close(fd);

	return 0;
}