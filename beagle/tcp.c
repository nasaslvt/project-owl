// Author: Peter Costescu
// NASA SLVT
// November, 2022

#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "tcp.h"

int create_tcp_socket()
{
    return socket(AF_INET, SOCK_STREAM, 0); // sys/socket.h
}

// creates a sockaddr_in struct and fills it
sockaddr_in create_server(int port, const char *address_string)
{
    sockaddr_in server = {AF_INET, htons(port), {inet_addr(address_string)}};
    return server;
}

// takes the c_str() of msg and writes it to socket_fd
// msg must not be more than 255 characters (256th is 0)
int write_string(int socket_fd, const char *msg)
{
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, msg, 255); // only copy 255 bytes to preserve null

    return write(socket_fd, buffer, 256);
}

// reads 255 bytes from the socket and assigns them to rcv
// this function blocks on read
int read_string(int socket_fd, char *rcv)
{
    char buffer[256];
    memset(buffer, 0, sizeof(char) * 256);

    int read_result = read(socket_fd, buffer, 255);

    if (read_result >= 0)
    {
        strcpy(rcv, buffer);
    }

    return read_result;
}

int read_buffer(int socket_fd, char *buffer, int n)
{
    memset(buffer, 0, sizeof(char) * n);
    int read_result = read(socket_fd, buffer, n - 1); 
    return read_result;
}

int write_buffer(int socket_fd, char *buffer, int n)
{
    int write_result = write(socket_fd, buffer, n); 
    return write_result;
}