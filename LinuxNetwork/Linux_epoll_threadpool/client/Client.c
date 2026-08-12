#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>

#define ARGS_CHECK(agrc, num) {if(argc != num){fprintf(stderr,"args error!\n");return -1;}}
#define ERROR_CHECK(ret, num, msg) {if(ret == num){perror(msg);return -1;}}

// v1.0 流式接收
//int recvFile(int sockfd)
//{
//	// 接收文件名
//	char filename[4096] = { 0 };
//	recv(sockfd, filename, sizeof(filename), 0);
//	
//	// 接收文件内容
//	int fd = open(filename, O_CREAT|O_RDWR, 0666);
//	char buf[1000] = { 0 };
//	ssize_t sret = revc(sockfd, buf, sizeof(buf), 0);
//
//	// 将接收的文件写入磁盘
//	write(fd, buf, sret);
//
//	return 0;
//}

// v2.0 包式接收小文件
//typedef struct train_s
//{
//	int length;// 协议
//	char data[1000];// 数据，char 数组不是表示字符串，可以存任何数据，数据上限是 1000
//} train_t;
//int recvFile(int sockfd)
//{
//	// 接收文件名
//	char filename[4096] = { 0 };
//	train_t train;
//	// 先收协议，后收数据
//	recv(sockfd, &train.length, sizeof(train.length), 0);
//	recv(sockfd, train.data, train.length, 0);
//	memcpy(filename, train.data, train.length);
//	
//	// 接收文件内容
//	recv(sockfd, &train.length, sizeof(train.length), 0);
//	recv(sockfd, train.data, train.length, 0);
//	int fd = open(filename, O_CREAT|O_RDWR, 0666);
//
//	// 将接收的文件写入磁盘
//	write(fd, train.data, train.length);
//
//	close(fd);
//
//	return 0;
//}

// v3.0  包式接收大文件
//typedef struct train_s
//{
//	int length;// 协议
//	char data[1000];// 数据，char 数组不是表示字符串，可以存任何数据，数据上限是 1000
//} train_t;
//int recvFile(int sockfd)
//{
//	// 接收文件名
//	char filename[4096] = { 0 };
//	train_t train;
//	// 先收协议，后收数据
//	// 因为会满足底层 MTU 要求，若 MTU < 1000，底层会多次流式发送数据，即半包问题
//	// 造成 recv 接收不足 1000 ，使得不能正确识别协议与数据，此时需添加 MSG_WAITALL
//	recv(sockfd, &train.length, sizeof(train.length), MSG_WAITALL);
//	recv(sockfd, train.data, train.length, MSG_WAITALL);
//	memcpy(filename, train.data, train.length);
//
//	int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0666);
//	while (1) {
//		// 接收文件内容
//		recv(sockfd, &train.length, sizeof(train.length), MSG_WAITALL);
//
//		if (train.length != 1000) {
//			printf("train.length = %d\n", train.length);
//		}
//
//		if (train.length == 0) {
//			break;
//		}
//		recv(sockfd, train.data, train.length, MSG_WAITALL);
//		
//		// 将接收的文件写入磁盘
//		write(fd, train.data, train.length);
//	}
//
//	close(fd);
//
//	return 0;
//}

// v4.0  包式接收大文件，加上进度条，可以接收指定大小的数据
// 接收指定大小的数据 
//int recvn(int sockfd, void* buf, int total)
//{
//	char* p = (char*)buf;
//	long cursize = 0;
//
//	while (cursize < total) {
//		ssize_t sret = recv(sockfd, p + cursize, total - cursize, 0);
//		if (sret == 0) {
//			return 1;
//		}
//		cursize += sret;
//	}
//
//	return 0;
//}
//
//typedef struct train_s
//{
//	int length;// 协议
//	char data[1000];// 数据，char 数组不是表示字符串，可以存任何数据，数据上限是 1000
//} train_t;
//int recvFile(int sockfd)
//{
//	// 接收文件名
//	char filename[4096] = { 0 };
//	train_t train;
//	// 先收协议，后收数据
//	// 因为会满足底层 MTU 要求，若 MTU < 1000，底层会多次流式发送数据，即半包问题
//	// 造成 recv 接收不足 1000 ，使得不能正确识别协议与数据，此时需添加 MSG_WAITALL
//	//recv(sockfd, &train.length, sizeof(train.length), MSG_WAITALL);
//	//recv(sockfd, train.data, train.length, MSG_WAITALL);
//	recvn(sockfd, &train.length, sizeof(train.length));
//	recvn(sockfd, train.data, train.length);
//	memcpy(filename, train.data, train.length);
//
//	// 接收文件长度
//	off_t filesize;
//	//recv(sockfd, &train.length, sizeof(train.length), MSG_WAITALL);
//	//recv(sockfd, train.data, train.length, MSG_WAITALL);
//	recvn(sockfd, &train.length, sizeof(train.length));
//	recvn(sockfd, train.data, train.length);
//	memcpy(filesize, train.data, train.length);
//	
//	off_t cursize = 0;// 当前接收文件大小
//	off_t lastsize = 0;// 上次接收文件大小
//	off_t slice = filesize / 10000;// 最小分片
//
//	int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0666);
//	while (1) {
//		// 接收文件内容
//		//recv(sockfd, &train.length, sizeof(train.length), MSG_WAITALL);
//		recvn(sockfd, &train.length, sizeof(train.length));
//
//		if (train.length == 0) {
//			break;
//		}
//		//recv(sockfd, &train.data, train.length, MSG_WAITALL);
//		recvn(sockfd, &train.data, train.length);
//
//		// 显示接收文件大小
//		cursize += train.length;
//		// 频繁显示进度条会影响性能，设置进度条显示条件
//		if (cursize - lastsize > slice) {
//			printf("5.2lf%%\r", cursize * 100.0 / filesize);// \r 回车，把光标移到开头会覆盖之前的显示
//			fflush(stdout);// \r 不会刷新缓冲区，此时手动刷新缓冲区
//			lastsize = cursize;
//		}
//		
//		// 将接收的文件写入磁盘
//		write(fd, train.data, train.length);
//	}
//
//	printf("100.00%%\n");
//
//	close(fd);
//
//	return 0;
//}

// v5.0  包式接收大文件，加上进度条，可以接收指定大小的数据，一次性接受数据 
// 接收指定大小的数据 
int recvn(int sockfd, void* buf, int total)
{
	char* p = (char*)buf;
	long cursize = 0;

	while (cursize < total) {
		ssize_t sret = recv(sockfd, p + cursize, total - cursize, 0);
		if (sret == 0) {
			return 1;
		}
		cursize += sret;
	}

	return 0;
}

typedef struct train_s
{
	int length;// 协议
	char data[1000];// 数据，char 数组不是表示字符串，可以存任何数据，数据上限是 1000
} train_t;
int recvFile(int sockfd)
{
	// 接收文件名
	char filename[4096] = { 0 };
	train_t train;
	// 先收协议，后收数据
	// 因为会满足底层 MTU 要求，若 MTU < 1000，底层会多次流式发送数据，即半包问题
	// 造成 recv 接收不足 1000 ，使得不能正确识别协议与数据，此时需添加 MSG_WAITALL
	//recv(sockfd, &train.length, sizeof(train.length), MSG_WAITALL);
	//recv(sockfd, train.data, train.length, MSG_WAITALL);
	recvn(sockfd, &train.length, sizeof(train.length));
	recvn(sockfd, train.data, train.length);
	memcpy(filename, train.data, train.length);

	// 接收文件长度
	off_t filesize;
	//recv(sockfd, &train.length, sizeof(train.length), MSG_WAITALL);
	//recv(sockfd, train.data, train.length, MSG_WAITALL);
	recvn(sockfd, &train.length, sizeof(train.length));
	recvn(sockfd, train.data, train.length);
	memcpy(&filesize, train.data, train.length);

	// mmap 映射
	int fd = open("file1", O_CREAT | O_RDWR | O_TRUNC, 0666);
	ftruncate(fd, filesize);// 先修改文件大小
	char* p = (char*)mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	ERROR_CHECK(p, MAP_FAILED, "mmap");

	// 接收文件
	recvn(sockfd, p, filesize);

	munmap(p, filesize);
	close(fd);

	return 0;
}

int main(int argc, char* argv[])
{
	ARGS_CHECK(agrc, 3);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	addr.sin_addr.s_addr = inet_addr(argv[1]);

	int ret = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
	ERROR_CHECK(ret, -1, "connect");

	recvFile(sockfd);

	close(sockfd);

	return 0;
}



