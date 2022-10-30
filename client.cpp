#include <iostream>
#include <string>
#include <strings.h> // bzero
//#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // read, write
#include <sstream>

using std::cout;
using std::endl;
using std::string;

int main(int argc, char *argv[])
{
    const int port = 9000;
    const char *address_string = "192.168.7.1";

    struct sockaddr_in server_address; // netinet/in.h
    int socket_fd;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0); // sys/socket.h
    if (socket_fd < 0)
    {
        cout << "Error: socket could not open correctly, socket_fd = " << socket_fd << endl;
        return -10 + socket_fd;
    }

    // struct server_address of type sockaddr_in has members sin_family, sin_port, and sin_addr
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port); // host to network byte order
    server_address.sin_addr.s_addr = inet_addr(address_string); // convert string to network order bytes
    
    cout << "Attempting to connect to " << address_string << endl;
    int connect_result = connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    if (connect_result < 0)
    {
        cout << "Error: connect returned " << connect_result << endl;
        return -20 + connect_result;
    }

    cout << "Connection succesful." << endl;

    char buffer[256];
    string msg;
    std::stringstream convert;
    for (int i = 0; ; i++)
    {
        bzero(buffer, 256);
        convert.str("");
        convert << i << '\n';
        msg = convert.str();

        cout << "Writing: " << msg << endl;
        int write_result = write(socket_fd, msg.c_str(), msg.size() + 1);
        if (write_result < 0)
        {
            cout << "Error: write returned " << write_result << endl;
        }

        int read_result = read(socket_fd, buffer, 255); 
        if (read_result < 0)
        {
            cout << "Error: read returned " << read_result << endl;
        }

        cout << "Read: " << string(buffer) << endl;
    }

    close(socket_fd); 
    return 0;
    
}
