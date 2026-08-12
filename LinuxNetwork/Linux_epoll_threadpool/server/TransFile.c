#include "ThreadPool.h"

// v1.0  流式发送
//int transFile(int netfd)
//{
//	// 发送文件名
//	char filename[] = "file1";
//	send(netfd, filename, 5);
//
//	// 读取磁盘文件
//	int fd = open(filename, O_RDWR);
//	char buf[1000] = { 0 };
//	ssize_t sret = read(fd, buf, sizeof(buf));
//
//	// 发送磁盘文件
//	send(netfd, buf, sret, 0);
//
//	return 0;
//} 

// v2.0 包式发送小文件
//typedef struct train_s
//{
//	int length;// 协议
//	char data[1000];// 数据，char 数组不是表示字符串，可以存任何数据，数据上限是 1000
//} train_t;
//int transFile(int netfd)
//{
//	// 发送文件名
//	train_t train;
//	char filename[] = "file1";
//	train.length = strlen(filename);
//	memcpy(train.data, filename, train.length);
//	send(netfd, &train, sizeof(train.length) + train.length);
//
//	// 读取磁盘文件
//	int fd = open(filename, O_RDWR);
//	ssize_t sret = read(fd, train.data, sizeof(train.data));
//	train.length = sret;
//
//	// 发送磁盘文件
//	send(netfd, &train, sizeof(train.length) + train.length, 0);
//
//	close(fd);
//
//	return 0;
//} 

// v3.0 包式发送大文件
//typedef struct train_s
//{
//	int length;// 协议
//	char data[1000];// 数据，char 数组不是表示字符串，可以存任何数据，数据上限是 1000
//} train_t;
//int transFile(int netfd)
//{
//	// 发送文件名
//	train_t train;
//	char filename[] = "file1";
//	train.length = strlen(filename);
//	memcpy(train.data, filename, train.length);
//	// 若客户端关闭，继续发送数据会产生 SIGPIPE，使得子线程退出为僵尸线程
//	// 父子 socket 一直可读，使 readySet 一直就绪，服务端崩溃
//	// 可以使用 signal 将 SIGPIPE 注册，也可以在 send 中添加 MSG_NOSIGNAL
//	send(netfd, &train, sizeof(train.length) + train.length,MSG_NOSIGNAL);
//
//	// 读取磁盘文件
//	int fd = open(filename, O_RDWR);
//	while (1) {
//		ssize_t sret = read(fd, train.data, sizeof(train.data));
//		train.length = sret;
//
//		// 发送磁盘文件
//		ssize_t sret1 = send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
//
//		// 判读是否发送完毕或发送失败，最后一次会发送 0，以表示结束
//		if (sret == 0 || sret1 == -1)
//		{
//			break;
//		}
//	}
//
//	close(fd);
//
//	return 0;
//}
 
//// v4.0 包式发送大文件，添加进度条
//typedef struct train_s
//{
//	int length;// 协议
//	char data[1000];// 数据，char 数组不是表示字符串，可以存任何数据，数据上限是 1000
//} train_t;
//int transFile(int netfd)
//{
//	// 发送文件名
//	train_t train;
//	char filename[] = "file1";
//	train.length = strlen(filename);
//	memcpy(train.data, filename, train.length);
//	// 若客户端关闭，继续发送数据会产生 SIGPIPE，使得子线程退出为僵尸线程
//	// 父子 socket 一直可读，使 readySet 一直就绪，服务端崩溃
//	// 可以使用 signal 将 SIGPIPE 注册，也可以在 send 中添加 MSG_NOSIGNAL
//	send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
//
//	// 读取磁盘文件
//	// 发送文件长度
//	int fd = open(filename, O_RDWR);
//	// 获取文件长度
//	struct stat statbuf;
//	fstat(fd, &statbuf);
//	train.length = sizeof(off_t);// off_t 长整型
//	memcpy(train.data, &statbuf.st_size, train.length);
//	send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
//	
//
//	while (1) {
//		ssize_t sret = read(fd, train.data, sizeof(train.data));
//		train.length = sret;
//
//		// 发送磁盘文件
//		ssize_t sret1 = send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
//
//		// 判读是否发送完毕或发送失败，最后一次会发送 0，以表示结束
//		if (sret == 0 || sret1 == -1)
//		{
//			break;
//		}
//	}
//
//	close(fd);
//
//	return 0;
//} 

// v4.1 包式发送大文件，添加进度条，添加零拷贝技术
//typedef struct train_s
//{
//	int length;// 协议
//	char data[1000];// 数据，char 数组不是表示字符串，可以存任何数据，数据上限是 1000
//} train_t;
//int transFile(int netfd)
//{
//	// 发送文件名
//	train_t train;
//	char filename[] = "file1";
//	train.length = strlen(filename);
//	memcpy(train.data, filename, train.length);
//	// 若客户端关闭，继续发送数据会产生 SIGPIPE，使得子线程退出为僵尸线程
//	// 父子 socket 一直可读，使 readySet 一直就绪，服务端崩溃
//	// 可以使用 signal 将 SIGPIPE 注册，也可以在 send 中添加 MSG_NOSIGNAL
//	send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
//
//	// 读取磁盘文件
//	// 发送文件长度
//	int fd = open(filename, O_RDWR);
//	// 获取文件总长度
//	struct stat statbuf;
//	fstat(fd, &statbuf);
//	train.length = sizeof(off_t);// off_t 长整型
//
//	// mmap 映射
//	char* p = (char*)mmap(NULL, statbuf.st_size, PROT_READ | RPOT_WRITE, MAP_SHARED, fd, 0);
//	ERROR_CHECK(p, MAP_FAILED, "mmap");
//	munmap(p, statbuf.st_size);
//	off_t cursize = 0;// 已发送的长度
//
//	while (1) {
//		if (cursize >= statbuf.st_size) {
//			break;
//		}
//
//		if (statbuf.st_size - cursize >= 1000) {
//			train.length = 1000;
//		}
//		else {
//			train.length = statbuf.st_size - cursize;
//		}
//
//		// 发送协议和数据
//		send(netfd, &train.length, sizeof(train.length), MSG_NOSIGNAL);
//		send(netfd, p + cursize, train.length, MSG_NOSIGNAL);// 每一次发送都是从映射区发送
//
//		cursize += train.length;
//	}
//
//	// 发送完毕标志
//	train.length = 0;
//	send(netfd, &train.length, sizeof(train.length), MSG_NOSIGNAL);
//
//	close(fd);
//
//	return 0;
//}

// v5.0 包式发送大文件，添加进度条，添加零拷贝技术，一次性发送数据
typedef struct train_s
{
	int length;// 协议
	char data[1000];// 数据，char 数组不是表示字符串，可以存任何数据，数据上限是 1000
} train_t;
int transFile(int netfd)
{
	// 发送文件名
	train_t train;
	char filename[] = "file1";
	train.length = strlen(filename);
	memcpy(train.data, filename, train.length);
	// 若客户端关闭，继续发送数据会产生 SIGPIPE，使得子线程退出为僵尸线程
	// 父子 socket 一直可读，使 readySet 一直就绪，服务端崩溃
	// 可以使用 signal 将 SIGPIPE 注册，也可以在 send 中添加 MSG_NOSIGNAL
	send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);

	// 读取磁盘文件
	// 发送文件长度
	int fd = open(filename, O_RDWR);
	// 获取文件总长度
	struct stat statbuf;
	fstat(fd, &statbuf);
	train.length = sizeof(off_t);// off_t 长整型
	memcpy(train.data, &statbuf.st_size, train.length);
	send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);

	// mmap 映射
	//char* p = (char*)mmap(NULL, statbuf.st_size, PROT_READ | RPOT_WRITE, MAP_SHARED, fd, 0);
	//ERROR_CHECK(p, MAP_FAILED, "mmap");
	
	// 发送全部文件
	//send(netfd, statbuf.st_size, MSG_NOSIGNAL);

	//munmap(p, statbuf.st_size);

	// 可以使用 sendfile，其原理就是 mmap 映射发送，可以节省代码量
	sendfile(netfd, fd, NULL, statbuf.st_size);

	close(fd);

	return 0;
}