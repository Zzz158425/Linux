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

// 主进程用来保存每个子进程的信息
enum worker_status
{
	FREE,
	BUSY
};
typedef struct workerdata_s {
	pid_t pid;// 子进程 pid
	enum worker_status status;// 子进程状态
	int pipesockfd;// 父子进程通信管道
}workerdata_t;

// 创建子进程
int makeWorker(int workerNum, workerdata_t* workerArr);

// 初始化连接
int tcpInit(const char* ip, const char* port, int* psockfd);

// 增加 epoll 监听
int epollAdd(int epfd, int fd);

// 删除 epoll 监听
int epollDel(int epfd, int fd);

// 跨进程通信
// 父进程接收到了 netfd，但是 netfd 应该给子进程，此时应进行不同进程之间的 UDP，exitFlag 为是否退出进程池
int sendfd(int sockfd, int fdtosend, int exitFlag);
int recvfd(int sockfd, int* pfdtorecv, int* pexitFlag);

// 子进程工作
int workLoop(int sockfd);

// 给客户端传输文件
int transFile(int netfd);