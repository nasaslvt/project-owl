// Author: Peter Costescu
// NASA SLVT
// November, 2022

#include <iostream>
#include <unistd.h>
#include <vector>
#include "tcp.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

const char FEND = 0xc0; 
const char DATA = 0x00;

// returns -1 if msg too big to fit in the buffer
int fill_buffer(char *buffer, unsigned int n, string msg)
{
    if (msg.size() + 3 > n)
        return -1;

    memset(buffer, 0, sizeof(char) * 256);
    buffer[0] = FEND;
    buffer[1] = DATA;
    memcpy(buffer + 2, msg.data(), msg.size());
    buffer[2 + msg.size()] = FEND;
    return 3 + msg.size();
}

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

    vector<string> messages;
    messages.push_back("KQ4DIB A1 B2 C3 D4 E5 F6 G7 H8");
    messages.push_back(" KQ4DIB A1 B2   ");
    messages.push_back("KQ4DIB   DD F E6 H8   ");
    messages.push_back("KQ4DIB    `4*&5987   ");
    messages.push_back("KQ4DIB TE *9 H$ test");
    messages.push_back("KQ4DIB  A1        H8");


    // 1 byte FEND, 1 byte DATA, call, msg, 1 byte FEND
    
    char buffer[256];
    
    for (unsigned int i = 0; i < messages.size(); i++)
    {
        string message = messages[i];
        cout << "Enter a character to send message \'" << message << '\'' << endl;
        char temp;
        std::cin >> temp;

        int num_write = fill_buffer(buffer, 256, message);
        if (num_write <= 0)
        {
            cout << "ERROR: num_write returned " << num_write << ". Message " << message << " likely could not be packed in buffer." << endl;
            continue;
        }

        cout << "Sending message." << endl;
        result = write_buffer(socket_fd, buffer, num_write);
        if (result <= 0)
            return error("Failed writing", result); 
    }
    sleep(2);
    close(socket_fd); 
    return 0;    
}
