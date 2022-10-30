#include <iostream>
//#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

using std::cout;

int main(int argc, char *argv[])
{
    const int port = 9000;

    struct sockaddr_in server_address; // netinet/in.h
    int socket_fd;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0); // sys/socket.h
    if (socket_fd < 0)
    {
        cout << "Error: socket could not open correctly, socket_fd = " << socket_fd << endl;
    }

    // struct server_address of type sockaddr_in has members sin_family, sin_port, and sin_addr
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port); // host to network byte order
    // member sin_addr of 
    server_address.sin_addr.s_addr
    return 0;
}