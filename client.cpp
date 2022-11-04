#include <iostream>
#include <string>
#include <cstring> // memset

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // read, write

using std::cout;
using std::endl;
using std::string;

// Returns value of call to socket
int create_tcp_socket()
{
    return socket(AF_INET, SOCK_STREAM, 0); // sys/socket.h
}

// creates a sockaddr_in struct and fills it
sockaddr_in create_server(int port, const char *address_string)
{
    struct sockaddr_in server = {AF_INET, htons(port), inet_addr(address_string)};
    return server;
}

// prints the msg and returns e
int error(string msg, int e)
{
    cout << "ERROR [" << e << "]: " << msg << endl;
    return e;
}

// takes the c_str() of msg and writes it to socket_fd
// msg must not be more than 255 characters (256th is 0)
int write_string(int socket_fd, string msg)
{
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, msg.c_str(), 255); // only copy 255 bytes to preserve null

    return write(socket_fd, buffer, 256);
}

// reads 255 bytes from the socket and assigns them to rcv
// this function blocks on read
int read_string(int socket_fd, string &rcv)
{
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    int read_result = read(socket_fd, buffer, 255); 
    if (read_result >= 0)
    {
        rcv.assign(buffer);
    }

    return read_result;
}

int main(int argc, char *argv[])
{
    const int port = 9000;
    const char *address_string = "192.168.7.1";
    int result = 0; // used to store results of calls

    int socket_fd = create_tcp_socket(); 
    if (socket_fd < 0)
        return error("Failed to create tcp socket", socket_fd);
    
    struct sockaddr_in server = create_server(port, address_string);//

    cout << "Attempting to connect socket to " << address_string << endl;
    result = connect(socket_fd, (struct sockaddr *) &server, sizeof(server));
    if (result < 0)
    {
        return error("Failed to connect", result);
    }

    cout << "Connection succesful." << endl;

    for (int i = 0; ; i++)
    {
        string msg = std::to_string(i);
        cout << "Writing: " << msg << endl;
        result = write_string(socket_fd, msg);
        if (result < 0)
            return error("Failed writing", result);

        string rcv;
        cout << "Reading" << endl;
        result = read_string(socket_fd, rcv);
        if (result < 0)
            return error("Failed reading", result);   
        cout << "Read: " << rcv << endl;     
    }

    close(socket_fd); 
    return 0;    
}
