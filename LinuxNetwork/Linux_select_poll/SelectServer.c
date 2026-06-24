#include<stdio.h>
#include<ctype.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<arpa/inet.h>

int main()
{
	// 创建监听的套接字
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		perror("socket");
		return -1;
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
		return -1;
	}

	// 监听
	ret = listen(lfd, 128);
	if (ret == -1)
	{
		perror("listen");
		return -1;
	}

	// 创建读事件监听集合
	fd_set redset; // 创建一个文件描述符集合
	FD_ZERO(&redset);// 初始化集合
	FD_SET(lfd, &redset);// 把监听套接字 lfd 加入读集合

	int maxfd = lfd;

	while (1)
	{
		fd_set tmp = redset;
		int ret = select(maxfd + 1, &tmp, NULL, NULL, NULL);//要检测的 fd 范围：maxfd + 1
		if (ret == -1)
		{
			perror("select");
			return -1;
		}

		// 如果 lfd 有事件，说明有新客户端连接
		if (FD_ISSET(lfd, &tmp))
		{
			// 接收客户端的连接
			int cfd = accept(lfd, NULL, NULL);
			if (cfd == -1)
			{
				perror("accept");
				return -1;
			}

			printf("new client connected, cfd = %d\n", cfd);
			
			FD_SET(cfd, &redset);
			maxfd = cfd > maxfd ? cfd : maxfd;
		}

		// 不管有没有新连接，都要检查已有客户端有没有发数据
		for (int i = 0; i <= maxfd; i++)
		{
			if (i != lfd && FD_ISSET(i, &tmp))
			{
				// 接收数据
				char buf[1024];

				int len = recv(i, buf, sizeof(buf), 0);
				if (len == -1)
				{
					perror("recv");
					return -1;
				}
				else if (len == 0)
				{
					printf("client disconnected...\n");
					FD_CLR(i, &redset);
					close(i);
					break;
				}

				printf("read buf = %s\n", buf);

				// 小写转大写
				for (int i = 0; i < len; i++)
				{
					buf[i] = toupper(buf[i]);
				}
				printf("after buf = %s\n", buf);

				// 发送数据
				ret = send(i, buf, strlen(buf) + 1, 0);
				if (ret == -1)
				{
					perror("send");
					return -1;
				}
			}
		}
	}

	// 关闭监听的文件描述符
	close(lfd);

	return 0;
}