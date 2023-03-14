// Author: Peter Costescu
// NASA SLVT
// November, 2022

#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct sockaddr_in sockaddr_in;

int create_tcp_socket();
sockaddr_in create_server(int port, const char *address_string);
int write_string(int socket_fd, const char * msg);
int write_buffer(int socket_fd, char *buffer, int n);
int read_string(int socket_fd, char *rcv);
int read_buffer(int socket_fd, char *buffer, int n);

#endif