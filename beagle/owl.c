#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcp.h"

#define KISSPORT 8001
#define LOCALHOST "127.0.0.1"

#define CAMERAPORT 8080
#define CAMERAIP "192.168.8.10"

int init_conn(int port, const char *addr) {

    int fd = create_tcp_socket();
    if (fd < 0) {
        fprintf(stderr, "%s : Failed to create tcp socket\n", addr);
        return -1;
    }

    sockaddr_in server = create_server(KISSPORT, LOCALHOST);

    if (connect(fd, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        fprintf(stderr, "%s : Failed to connect\n", addr);
        return -1;
    }
    printf("%s : Connection succesful.\n", addr);

    return fd;
}

int main() {

    int res = 0;

    int camera_socket_fd = init_conn(CAMERAPORT, CAMERAIP);
    int radio_socket_fd = init_conn(KISSPORT, LOCALHOST);

    char request[256];
    char response[256];

    strcpy(request, "GET / HTTP/1.1\r\nHost: 192.168.8.10:8000\r\n\r\n");
    write_string(camera_socket_fd, request);
    read_string(camera_socket_fd, response);

    while (1) {
        res = read_buffer(radio_socket_fd, response, 256);
        if (res < 0) {
            //fprintf(stderr, "Failed reading\n");
        }

        //TODO
    }

    close(radio_socket_fd);
    close(camera_socket_fd);
    return 0;
}