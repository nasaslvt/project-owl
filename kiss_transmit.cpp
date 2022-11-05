#include <iostream>
#include <unistd.h>
#include "tcp.h"

using std::cout;
using std::endl;
using std::string;


int main(int argc, char *argv[])
{
    const int port = 8001;
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

    const char *call = "KQ4DIB"; // 6 bytes
    const char *msg = "NASA SLVT"; // 9 bytes
    const char FEND = 0xc0; 
    const char DATA = 0x00;

    // 1 byte FEND, 1 byte DATA, 6 byte call, 9 byte msg, 1 byte FEND
    // total 18 bytes
    // message looks like following (ASCII chars used instead of hex for message body)
    // remember, 0x20 is space
    // 0xc0 0x00 K Q 4 D I B N A S A 0x20 S L V T 0xc0

    char buffer[256];
    memset(buffer, 0, sizeof(char) * 256);

    buffer[0] = FEND;
    buffer[1] = DATA;
    memcpy(buffer + 2, call, 6);
    memcpy(buffer + 8, msg, 9);
    buffer[17] = FEND;
    
    for (int i = 0; ; i++)
    {
        cout << "Attempting to write" << endl;
        result = write_buffer(socket_fd, buffer, 18);
        if (result < 0)
            return error("Failed writing", result); 

        sleep(10);
    }

    close(socket_fd); 
    return 0;    
}
