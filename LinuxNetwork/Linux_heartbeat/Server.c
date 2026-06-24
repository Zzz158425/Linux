#include "Socket.h"
#include "Clientlist.h"

pthread_mutex_t mutex;

void* parseRecvMessage(void* arg)
{
	struct ClientInfo* info = (struct ClientInfo*)arg;

	while (1)
	{
		char* buffer;
		enum Type t;

		// 接收数据
		int len = recvMessage(info->fd, &buffer, &t);
		if (buffer == NULL)
		{
			printf("fd = %d, The communication child thread has exited...\n", info->fd);
			pthread_exit(NULL);
		}
		else
		{
			if (t == Heart)
			{
				printf("heartbeat packet: %s\n", buffer);

				pthread_mutex_lock(&mutex);

				info->count = 0;

				pthread_mutex_unlock(&mutex);

				sendMessage(info->fd, buffer, len, Heart);
			}
			else
			{
				printf("data packet: %s\n", buffer);

				const char* pt = "Received";
				sendMessage(info->fd, pt, strlen(pt), Message);
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
	struct ClientInfo* head = (struct ClientInfo*)arg;

	struct ClientInfo* p = NULL;

	while (1)
	{
		p = head->next;
		while (p)
		{
			pthread_mutex_lock(&mutex);

			struct ClientInfo* next = p->next;// 记录 p 下一个节点的位置，防止当前节点被删除后，再访问已经释放的 p->next

			p->count++;// 默认没有收到服务器回复的心跳包数据

			printf("fd = %d, count = %d\n", p->fd, p->count);

			if (p->count > 5)
			{
				int fd = p->fd;
				pthread_t pid = p->pid;

				// 认为客户端与服务器断开了连接
				printf("The client fd = %d,  has disconnected from the server", p->fd);
				
				// 释放套接字资源，使该线程退出，移除对应节点
				close(fd);
				pthread_cancel(pid);
				removeNode(head, fd);
			}

			pthread_mutex_unlock(&mutex);

			p = next;
		}

		sleep(3);
	}

	return NULL;
}

int main()
{
	// 创建服务器套接字、绑定、监听
	unsigned short port = 10000;
	int lfd = initSocket();
	setListen(lfd, port);

	// 创建链表
	struct ClientInfo* head = createList();

	pthread_mutex_init(&mutex, NULL);

	// 添加心跳包子线程
	pthread_t pid1;
	pthread_create(&pid1, NULL, heartBeat, head);

	while (1)
	{
		int sockfd = acceptConnect(lfd, NULL);
		if (sockfd == -1)
		{
			continue;
		}

		struct ClientInfo* node = prependNode(head, sockfd);

		// 创建接收数据的子线程
		pthread_create(&node->pid, NULL, parseRecvMessage, node);
		pthread_detach(node->pid);

	}

	pthread_join(pid1, NULL);
	pthread_mutex_destroy(&mutex);

	close(lfd);

	return 0;
}