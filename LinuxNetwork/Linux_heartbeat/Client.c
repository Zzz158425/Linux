#include "Socket.h"

pthread_mutex_t mutex;

// 信息结构体
struct FdInfo
{
	int fd;
	int count; // 记录有多少次没有收到服务器回复的心跳包数据
};

void* parseRecvMessage(void* arg)
{
	struct FdInfo* info = (struct FdInfo*)arg;

	while (1)
	{
		char* buffer;
		enum Type t;

		// 接收数据
		recvMessage(info->fd, &buffer, &t);
		if (buffer == NULL)
		{
			continue;
		}
		else
		{
			if (t == Heart)
			{
				printf("heartbeat packet: %s\n", buffer);

				pthread_mutex_lock(&mutex);

				info->count = 0;

				pthread_mutex_unlock(&mutex);
			}
			else
			{
				printf("data packet: %s\n", buffer);
			}

			free(buffer);
		}
	}

	return NULL;
}

void* heartBeat(void* arg)
{
	// 1.发送心跳包数据
	// 2.检测心跳包，看是否能收到服务器回复的数据
	struct FdInfo* info = (struct FdInfo*)arg;

	while (1)
	{
		pthread_mutex_lock(&mutex);

		info->count++;// 默认没有收到服务器回复的心跳包数据

		printf("fd = %d, count = %d\n", info->fd, info->count);

		if (info->count > 5)
		{
			// 认为客户端与服务器断开了连接
			printf("The client has disconnected from the server");

			// 释放套接字资源，退出客户端程序
			close(info->fd);

			exit(0);// exit 结束整个进程
		}

		pthread_mutex_unlock(&mutex);

		sendMessage(info->fd, "hello", 5, Heart);

		sleep(3);
	}

	return NULL;
}

int main()
{
	// 初始化信息结构体
	struct FdInfo info;
	info.count = 0;
	info.fd = initSocket();
	
	// 连接服务器
	unsigned short port = 10000;
	const char* ip = "192.168.248.131";// 127.0.0.1
	connectToHost(info.fd, port, ip);

	pthread_mutex_init(&mutex, NULL);

	// 创建接收数据的子线程
	pthread_t pid;
	pthread_create(&pid, NULL, parseRecvMessage, &info);

	// 添加心跳包子线程
	pthread_t pid1;
	pthread_create(&pid1, NULL, heartBeat, &info);

	while (1)
	{
		const char* data = "hello, big bing.....";

		// 发送数据
		sendMessage(info.fd, data, strlen(data), Message);
		sleep(2);
	}

	pthread_join(pid, NULL);
	pthread_join(pid1, NULL);
	pthread_mutex_destroy(&mutex);

	return 0;
}