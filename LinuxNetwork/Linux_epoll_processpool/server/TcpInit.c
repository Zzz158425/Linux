#include "Head.h"

int tcpInit(const char* ip, const char* port, int* psockfd)
{
	// socket
	*psockfd = socket(AF_INET, SOCK_STREAM, 0);

	// 设置 socket 的底层属性，开启端口地址复用
	int reuse = 1;
	int ret = setsockopt(*psockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	ERROR_CHECK(ret, -1, "setsockopt");
	
	// ip 与 端口
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = inet_addr(ip);
	
	// bind
	ret = bind(*psockfd, (struct sockaddr*)&addr, sizeof(addr));
	ERROR_CHECK(ret, -1, "bind");

	// listen
	listen(*psockfd, 128);

	return 0;
}