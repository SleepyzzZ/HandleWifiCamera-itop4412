#ifndef _NET_TRANS_H__
#define _NET_TRANS_H__

int init_tcp_socket(int *socket_fd, unsigned short port);

int init_tcp_client_socket(int *socket_fd, in_addr_t server_ip_addr, unsigned short port);

int tcp_send_data(int socket_fd, char *buff, int buff_len);

int udp_send_data(int socket_fd, char *buff, int buff_len, struct sockaddr* addr_client, int client_addr_len);

int tcp_recv_data(int socket_fd, char *buff, int buff_len, int flag);



#endif