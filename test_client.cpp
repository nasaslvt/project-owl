#include <iostream>
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
