#ifndef _SOCKET_H
#define _SOCKET_H

// ---------------服务器---------------
// 绑定 + 监听
int setListen(int lfd, unsigned short port);
int acceptConn(int lfd, struct sockaddr_in* addr);

// 等待客户端连接
int acceptConn(int fld, struct sockaddr_in* addr);

// ---------------客户端---------------
int connectToHost(int fd, const char* ip, unsigned short port);

// ---------------公用---------------
int createSocket();
int sendMsg(int fd, const char* msg, int len);
int recvMsg(int cfd, char** msg);
int closeSocket(int fd);

#endif
