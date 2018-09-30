#ifndef __NET_SOCKET_H__
#define __NET_SOCKET_H__

#include <arpa/inet.h>

// TCP
int tcp_get_socket();
int tcp_get_bind_socket(const char *ip, unsigned short port);
int tcp_connect(int sockfd, const char *ip, unsigned short port);
int tcp_listen(int sockfd, int backlog);
int tcp_accept(int sockfd, struct sockaddr_in *addr);

int tcp_recv(int sockfd, void *buff, int len);
int tcp_nowait_recv(int sockfd, void *buff, int len);
int tcp_waitall_recv(int sockfd, void *buff, int len);
int tcp_clear_recv(int sock);

int tcp_send(int sockfd, void *buff, int len);

// UDP
int udp_get_socket();
int udp_get_bind_socket(const char *ip, unsigned short port);
int udp_recvfrom(int sockfd, void *buff, int len, struct sockaddr_in *addr);
int udp_sendto(int sockfd, const char *ip, unsigned short port, void *buff, int len);

int socket_bind(int sockfd, const char *ip, unsigned short port);
int socket_close(int sock);

int set_socket_timeout(int sockfd, int sec);

#endif
