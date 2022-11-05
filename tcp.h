// Author: Peter Costescu
// NASA SLVT
// November, 2022

#include <string>
#include <cstring> // memset
#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // read, write

int create_tcp_socket();
sockaddr_in create_server(int port, const char *address_string);
int error(std::string msg, int e);
int write_string(int socket_fd, std::string msg);
int write_buffer(int socket_fd, char *buffer, int n);
int read_string(int socket_fd, std::string &rcv);
int read_buffer(int socket_fd, char *buffer, int n);