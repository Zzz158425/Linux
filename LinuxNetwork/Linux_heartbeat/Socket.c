#include "Socket.h"


// 初始化一个套接字
int initSocket()
{
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		perror("lfd");
		return -1;
	}

	return lfd;
}

// 初始化 sockaddr 结构体
void initSockaddr(struct sockaddr* addr, unsigned short port, const char* ip)
{
	struct sockaddr_in* addrin = (struct sockaddr_in*)addr;
	addrin->sin_family = AF_INET;
	addrin->sin_port = htons(port);
	addrin->sin_addr.s_addr = inet_addr(ip);
}

// 设置监听与绑定
int setListen(int lfd, unsigned short port)
{
	struct sockaddr addr;
	initSockaddr(&addr, port, "0.0.0.0");// INADDR_ANY = 0.0.0.0

	// 设置端口复用
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	int ret = bind(lfd, &addr, sizeof(addr));
	if (ret == -1)
	{
		perror("bind");
		return -1;
	}

	ret = listen(lfd, 128);
	if (ret == -1)
	{
		perror("listen");
		return -1;
	}

	return 0;
}

// 接收客户端连接
int acceptConnect(int lfd, struct sockaddr* addr)
{
	int connfd;

	if (addr == NULL)
	{
		connfd = accept(lfd, NULL, NULL);
	}
	else
	{
		socklen_t len = sizeof(struct sockaddr);
		connfd = accept(lfd, addr, &len);
	}
	if (connfd == -1)
	{
		perror("accept");
		return -1;
	}

	return connfd;
}

// 连接服务器
int connectToHost(int fd, unsigned short port, const char* ip)
{
	struct sockaddr addr;
	initSockaddr(&addr, port, ip);

	int ret = connect(fd, &addr, sizeof(addr));
	if (ret == -1)
	{
		perror("connect");
		return -1;
	}

	return 0;
}

// 读出指定的字节数
int readn(int fd, char* buffer, int size)
{
	int left = size;// 剩余未读的字节数
	int readBtyes = 0;// 已经读出的字节数
	char* ptr = buffer;

	while (left)
	{
		readBtyes = read(fd, ptr, left);
		if (readBtyes == -1)
		{
			// 因信号中断而发送失败
			if (errno == EINTR)
			{
				readBtyes = 0;
			}
			else
			{
				perror("write");
				return -1;
			}
		}
		// 对端主动断开了连接
		else if (readBtyes == 0)
		{
			printf("The peer actively closed the connection");
			return -1;
		}

		ptr += readBtyes;
		left -= readBtyes;
	}

	return size - left;
}

// 写入指定的字节数
int writen(int fd, const char* buffer, int length)
{
	int left = length;// 剩余写入的字节数
	int writeBtyes = 0;// 已经写入的字节数
	const char* ptr = buffer;

	while (left)
	{
		writeBtyes = write(fd, ptr, left);
		if (writeBtyes <= 0)
		{
			// 因信号中断而发送失败
			if (errno == EINTR)
			{
				writeBtyes = 0;
			}
			else
			{
				perror("write");
				return -1;
			}
		}
		ptr += writeBtyes;
		left -= writeBtyes;
	}

	return length;
}

// 发送数据
bool sendMessage(int fd, const char* buffer, int length, enum Type t)
{
	int dataLen = length + 1 + sizeof(int);// 数据大小 + 数据类型大小 + 报头大小
	// 申请堆内存，存放要发送的数据（数据 + 数据类型 + 报头）
	char* data = (char*)malloc(dataLen);
	if (data == NULL)
	{
		return false;
	}

	// 将数据写入 data
	int netlen = htonl(length + 1);
	memcpy(data, &netlen, sizeof(int));// 报头
	char ch = t == Heart ? 'H' : 'M';
	memcpy(data + sizeof(int), &ch, sizeof(char));// 数据类型
	memcpy(data + sizeof(int) + 1, buffer, length);// 数据

	// 发送
	int ret = writen(fd, data, dataLen);

	free(data);

	return ret == dataLen;
}

// 接收数据
int recvMessage(int fd, char** buffer, enum Type* t)
{
	// 接收报头
	int dataLen = 0;
	int ret = readn(fd, (char*)&dataLen, sizeof(int));// ----------
	if (ret == -1)
	{
		*buffer = NULL;
		return -1;
	}
	dataLen = ntohl(dataLen);

	// 接收数据类型
	char ch;
	readn(fd, &ch, 1);
	*t = ch == 'H' ? Heart : Message;

	// 接收数据
	char* tmpbuf = (char*)calloc(dataLen, sizeof(char));// calloc 会自动初始化开辟的内存
	if (tmpbuf == NULL)
	{
		*buffer = NULL;
		return -1;
	}
	ret = readn(fd, tmpbuf, dataLen - 1);
	if (ret != dataLen - 1)
	{
		free(tmpbuf);
		*buffer = NULL;
		return -1;
	}
	*buffer = tmpbuf;

	return ret;
}
