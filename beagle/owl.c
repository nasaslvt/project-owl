#include <stdio.h>
#include <stdlib.h>

#include "tcp.h"

#define KISSPORT 8001
#define LOCALHOST "127.0.0.1"

int main() {

    int res = 0;

    int socket_fd = create_tcp_socket();
    if (socket_fd < 0) {
        fprintf(stderr, "Failed to create tcp socket\n");
        return 1;
    }

    sockaddr_in server = create_server(KISSPORT, LOCALHOST);

    printf("Attempting to connect socket to %s\n", address_string);
    result = connect(socket_fd, (struct sockaddr *) &server, sizeof(server));
    if (result < 0)
    {
        fprintf(stderr, "Failed to connect\n");
        return 1;
    }
    fprintf("Connection succesful.\n");

    char buffer[256];

    while (1) {
        printf("Waiting for data...\n");
        result = read_buffer(socket_fd, buffer, 256);
        if (result < 0) {
            fprintf(stderr, "Failed reading");
        }

        //TODO
    }

    close(socket_fd); 
    return 0;
}