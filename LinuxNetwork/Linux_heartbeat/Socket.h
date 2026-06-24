#pragma once
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

// 数据包类似
// 数据包类型：'H'：心跳包，'M'：数据包
// 数据包格式：数据长度|数据包类型|数据块
//            int     char      char*   
//            4 字节  1 字节     N 字节
enum Type{Heart, Message};

// 初始化一个套接字
int initSocket();

// 初始化 sockaddr 结构体
void initSockaddr(struct sockaddr* addr, unsigned short port, const char* ip);

// 设置监听与绑定
int setListen(int lfd, unsigned short port);

// 接收客户端连接
int acceptConnect(int lfd, struct sockaddr* addr);

// 连接服务器
int connectToHost(int fd, unsigned short port, const char* ip);

// 读出指定的字节数
int readn(int fd, char* buffer, int size);

// 写入指定的字节数
int writen(int fd, const char* buffer, int length);

// 发送数据
bool sendMessage(int fd, const char* buffer, int length, enum Type t);

// 接收数据
int recvMessage(int fd, char** buffer, enum Type* t);