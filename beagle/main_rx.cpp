// Author: Peter Costescu
// NASA SLVT
// November 5, 2022

#include <iostream>
#include <vector>
#include <iomanip>
#include <cctype>
#include "tcp.h"
#include "parser.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::stringstream;


int main(int argc, char *argv[])
{
    const int port = 8001;
    const char *address_string = "127.0.0.1";
    int result = 0; // used to store results of calls

    int socket_fd = create_tcp_socket(); 
    if (socket_fd < 0)
        return error("Failed to create tcp socket", socket_fd);
    
    struct sockaddr_in server = create_server(port, address_string);
    
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

        string callsign;
        vector<Command> cmd = parse_buffer(buffer, 256, callsign);
        cout << "Callsign: " << callsign << endl;
        cout << "Command sequence: ";
        for (Command c : cmd)
        {
            cout << token_from_command(c) << ' ';
        }
        cout << endl;

    }

    close(socket_fd); 
    return 0;    
}
