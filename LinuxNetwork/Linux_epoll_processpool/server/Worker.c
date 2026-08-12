#include "Head.h"

// 创建子进程
int makeWorker(int workerNum, workerdata_t* workerArr)
{
	for (int i = 0; i < workerNum; i++) {
		// 创建父子线程通信管道
		int pipefd[2];
		socketpair(AF_LOCAL, SOCK_STREAM, 0, pipefd);

		pid_t pid = fork();
		if (pid == 0) {
			// 关闭一个子进程管道
			close(pipefd[1]);

			// 让子线程工作
			workLoop(pipefd[0]);
		}
		// 关闭一个父进程管道
		close(pipefd[0]);

		// 父进程调用 fork，获取了创建的子进程的 pid
		workerArr[i].status = FREE;
		workerArr[i].pid = pid;
		workerArr[i].pipesockfd = pipefd[1];
		printf("i = %d, pid = %d, pipefd = %d\n", i, pid, pipefd[1]);
	}
	return 0;
}

// 子线程工作
int workLoop(int sockfd)
{
	while (1) {
		// 接收任务
		int netfd;
		int exitFlag;

		recvfd(sockfd, &netfd, &exitFlag);

		//exitFlag 为 1，自行退出
		if (exitFlag == 1) {
			printf("stop work!\n");
			exit(0);
		}

		printf("start transFile work!\n");
		transFile(netfd);
		printf("start transFile over!\n");
		
		// 任务完成，向父进程发送自己的 pid
		pid_t pid = getpid();
		send(sockfd, &pid, sizeof(pid), 0);
		
		close(netfd);
	}
}



