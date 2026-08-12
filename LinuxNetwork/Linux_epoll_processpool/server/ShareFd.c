#include "Head.h"

// 跨进程通信
int sendfd(int sockfd, int fdtosend, int exitFlag)
{
	struct msghdr hdr;
	bzero(&hdr, sizeof(hdr)); // name -> NULL namelen -> 0

	// 正文，必须要有
	struct iovec vec[1];// 数组记录离散区域
	vec[0].iov_base = &exitFlag;// 第一个离散碎片的首地址，进程池退出标志
	vec[0].iov_len = sizeof(int);
	hdr.msg_iov = vec;
	hdr.msg_iovlen = 1;

	// 控制字部分，记录要发送的内核数据结构
	struct cmsghdr* pcmsg = (struct cmsghdr*)malloc(CMSG_LEN(sizeof(int)));
	pcmsg->cmsg_len = CMSG_LEN(sizeof(int));
	pcmsg->cmsg_level = SOL_SOCKET;
	pcmsg->cmsg_type = SCM_RIGHTS;
	// 通过 pcmsg 得到 data 首地址，强转为 int*，再解引用
	*(int*)CMSG_DATA(pcmsg) = fdtosend;
	hdr.msg_control = pcmsg;// 将控制字段的信息放入 hdr 中
	hdr.msg_controllen = CMSG_LEN(sizeof(int));
	
	// 发送
	int ret = sendmsg(sockfd, &hdr, 0);
	ERROR_CHECK(ret, -1, "sendmsg");

	return 0;
}
int recvfd(int sockfd, int* pfdtorecv, int* pexitFlag)
{
	struct msghdr hdr;
	bzero(&hdr, sizeof(hdr)); // name -> NULL namelen -> 0

	// 正文，必须要有
	char buf[6] = { 0 };// 正文数据
	struct iovec vec[1];// 数组记录离散区域
	vec[0].iov_base = pexitFlag;// 第一个离散碎片的首地址，进程池退出标志
	vec[0].iov_len = sizeof(int);
	hdr.msg_iov = vec;
	hdr.msg_iovlen = 1;

	// 控制字部分，记录要发送的内核数据结构
	struct cmsghdr* pcmsg = (struct cmsghdr*)malloc(CMSG_LEN(sizeof(int)));
	pcmsg->cmsg_len = CMSG_LEN(sizeof(int));
	pcmsg->cmsg_level = SOL_SOCKET;
	pcmsg->cmsg_type = SCM_RIGHTS;
	// 通过 pcmsg 得到 data 首地址，强转为 int*，再解引用
	hdr.msg_control = pcmsg;// 将控制字段的信息放入 hdr 中
	hdr.msg_controllen = CMSG_LEN(sizeof(int));

	// 接收
	int ret = recvmsg(sockfd, &hdr, 0);
	ERROR_CHECK(ret, -1, "recvmsg");
	printf("exitFlag = %d, fd = %d\n", *pexitFlag, *(int*)CMSG_DATA(pcmsg));
	*pfdtorecv = *(int*)CMSG_DATA(pcmsg);

	return 0;
}