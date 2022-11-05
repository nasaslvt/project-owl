// Author: Peter Costescu
// NASA SLVT
// November, 2022

#include <iostream>
#include <iomanip>
#include <cctype>
#include "tcp.h"

using std::cout;
using std::endl;
using std::string;


int main(int argc, char *argv[])
{
    const int port = 8001;
    const char *address_string = "127.0.0.1";
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

    char buffer[256];

    for (int i = 0; ; i++)
    {
        cout << "Waiting for data..." << endl;
        result = read_buffer(socket_fd, buffer, 256);
        if (result < 0)
            return error("Failed reading", result);  

        for (int i = 0; i < 256; i++)
        {
            cout << std::hex << std::setfill('0') << std::setw(2) << (int) buffer[i] << " ";
            if (i != 0 && buffer[i] == 0xc0)
                break;
        }
        cout << std::endl;

        for (int i = 0; i < 256; i++)
        {
            if (isprint(buffer[i]))
                cout << buffer[i] << "  ";
            else
                cout << "_  ";

            if (i != 0 && buffer[i] == 0xc0)
                break;
        }

        cout << std::endl << std::endl;
    }

    close(socket_fd); 
    return 0;    
}
