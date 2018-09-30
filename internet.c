#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include "internet.h"

// 获取TCP套接字
int tcp_get_socket()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( sockfd == -1 ) 
    {
        return -1;
    }

    return sockfd;
}

// 获取TCP绑定套接字
int tcp_get_bind_socket(const char *ip, unsigned short port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( sockfd == -1 ) 
    {
        return -1;
    }

    if ( socket_bind(sockfd, ip, port) < 0 )
    {
        return -1;
    }

    return sockfd;
}

// 设置套接字超时
int set_socket_timeout(int sockfd, int sec)
{
    struct timeval tv_out;
    tv_out.tv_sec = sec;
    tv_out.tv_usec = 0;
    if ( setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out)) < 0 )
    {
        return -1;
    }
    return 0;
}

// 套接字关联
int socket_bind(int sockfd, const char *ip, unsigned short port)
{
    int on = 1;	
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

    struct sockaddr_in addr={0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    socklen_t size = sizeof(struct sockaddr_in);

    return bind(sockfd, (struct sockaddr *)&addr, size);
}

int socket_close(int sockfd)
{
    return close(sockfd);
}

// 连接请求监听
int tcp_listen(int sockfd, int backlog)
{
    return listen(sockfd, backlog);
}

// 接收连接请求
int tcp_accept(int sockfd, struct sockaddr_in *addr)
{
	socklen_t size = sizeof(struct sockaddr_in);

    return accept(sockfd, (struct sockaddr *)addr, &size);
}

// 连接
int tcp_connect(int sockfd, const char *ip, unsigned short port)
{
	struct sockaddr_in addr={0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	socklen_t size = sizeof(struct sockaddr_in);

    if ( connect(sockfd, (struct sockaddr *)&addr, size) < 0 ) 
    {
        return -1;
    }

    return 0;
}

// 接收数据
int tcp_recv(int sockfd, void *buff, int len)
{
    return recv(sockfd, buff, len, 0);
}

// 非阻塞接收数据
int tcp_nowait_recv(int sockfd, void *buff, int len)
{
    return recv(sockfd, buff, len, MSG_DONTWAIT);
}

// 接收固定长度数据
int tcp_waitall_recv(int sockfd, void *buff, int len)
{
    return recv(sockfd, (unsigned char *)buff, len, MSG_WAITALL);
}

// 清除Recv缓存
int tcp_clear_recv(int sockfd)
{
    unsigned char buff[128] = {0};

    while (1)
    {
        int ret = recv(sockfd, buff, 127, MSG_DONTWAIT);
        if ( ret < 127 )
        {
            return -1;
        }
    }

    return 0;
}

// 发送数据
int tcp_send(int sockfd, void *buff, int len)
{
    return send(sockfd, buff, len, 0);
}

// 获取UDP套接字
int udp_get_socket()
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd == -1 ) 
    {
        return -1;
    }

    return sockfd;
}

// 获取UDP绑定套接字
int udp_get_bind_socket(const char *ip, unsigned short port)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd == -1 ) 
    {
        return -1;
    }

    if ( socket_bind(sockfd, ip, port) < 0 )
    {
        return -1;
    }

    return sockfd;
}

// 接收UDP数据
int udp_recvfrom(int sockfd, void *buff, int len, struct sockaddr_in *addr)
{
    if ( addr != NULL )
    {
        socklen_t size = sizeof(struct sockaddr_in);
        return recvfrom(sockfd, buff, len, 0, (struct sockaddr *)addr, &size);
    }

    return recvfrom(sockfd, buff, len, 0, NULL, NULL);
}

// 发送UDP数据
int udp_sendto(int sockfd, const char *ip, unsigned short port, void *buff, int len)
{
	struct sockaddr_in addr={0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	socklen_t size = sizeof(struct sockaddr_in);

    return sendto(sockfd, buff, len, 0, (struct sockaddr *)&addr, size);
}

// 关闭套接字
int CloseSock(int sockfd)
{
    return close(sockfd);
}

