// Author: Peter Costescu
// NASA SLVT
// November 5, 2022

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cctype>
#include "tcp.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::stringstream;

// A1 B2 C3 D4 E5 F6 G7 H8
enum class Command {RIGHT_60, LEFT_60, TAKE_PICTURE, MODE_GRAYSCALE, MODE_COLOR, PICTURE_180, SPECIAL_FILTER, REMOVE_FILTER, UNDEFINED};
enum class KState {NO_FRAME, FRAME_START, WHITESPACE_INITIAL, CALLSIGN, WHITESPACE, TOKEN};

string to_string(Command cmd)
{
    string s;
    switch (cmd)
    {
        case Command::RIGHT_60:
            s = "RIGHT_60";
            break;
        case Command::LEFT_60:
            s = "LEFT_60";
            break;
        case Command::TAKE_PICTURE:
            s = "TAKE_PICTURE";
            break;
        case Command::MODE_GRAYSCALE:
            s = "MODE_GRAYSCALE";
            break;
        case Command::MODE_COLOR:
            s = "MODE_COLOR";
            break;
        case Command::PICTURE_180:
            s = "PICTURE_180";
            break;
        case Command::SPECIAL_FILTER:
            s = "SPECIAL_FILTER";
            break;
        case Command::REMOVE_FILTER:
            s = "REMOVE_FILTER";
            break;
        case Command::UNDEFINED:
            s = "UNDEFINED";
            break;
        default:
            s = "UNKNOWN";
            break;
    }

    return s;
}

Command command_from_token(string token)
{
    Command cmd = Command::UNDEFINED;
    if (token == "A1")
        cmd = Command::RIGHT_60;
    if (token == "B2")
        cmd = Command::LEFT_60;
    if (token == "C3")
        cmd = Command::TAKE_PICTURE;
    if (token == "D4")
        cmd = Command::MODE_GRAYSCALE;
    if (token == "E5")
        cmd = Command::MODE_COLOR;
    if (token == "F6")
        cmd = Command::PICTURE_180;
    if (token == "G7")
        cmd = Command::SPECIAL_FILTER;
    if (token == "H8")
        cmd = Command::REMOVE_FILTER;
    
    return cmd;
}

string token_from_command(Command cmd)
{
    string token;
    switch (cmd)
    {
        case Command::RIGHT_60:
            token = "A1";
            break;
        case Command::LEFT_60:
            token = "B2";
            break;
        case Command::TAKE_PICTURE:
            token = "C3";
            break;
        case Command::MODE_GRAYSCALE:
            token = "D4";
            break;
        case Command::MODE_COLOR:
            token = "E5";
            break;
        case Command::PICTURE_180:
            token = "F6";
            break;
        case Command::SPECIAL_FILTER:
            token = "G7";
            break;
        case Command::REMOVE_FILTER:
            token = "H8";
            break;
        default:
            token = "XX";
            break;
    }

    return token;
}

// this function parses a KISS frame stored in the buffer
// returns vector of command enums, writes to the callsign reference
// char buffer must be allocated to n bytes.
// callsign will be cleared and the callsign in the frame will be written to it
vector<Command> parse_buffer(char *buffer, int n, string &callsign)
{
    stringstream token;
    KState state = KState::NO_FRAME;
    vector<Command> cmds;
    callsign = "";
    bool finished = false;

    for (int i = 0; i < n; i++)
    {
        if (finished)
            break;

        char c = buffer[i];
        switch (state)
        {
            case KState::NO_FRAME:
                if (c == 0xC0)
                    state = KState::FRAME_START;
                else
                    finished = true;
                break;

            case KState::FRAME_START:
                if (c == 0x00)
                    state = KState::WHITESPACE_INITIAL;
                else
                    finished = true;
                break;
            
            case KState::WHITESPACE_INITIAL:
                if (c == 0xC0)
                    finished = true;
                else if (isgraph(c))
                {
                    token << c;
                    state = KState::CALLSIGN;
                }
                else
                    state = KState::WHITESPACE_INITIAL;
                break;

            case KState::CALLSIGN:
                if (c == 0xC0)
                {
                    finished = true;
                    callsign = token.str();
                    token.clear();
                    token.str("");
                }
                else if (isgraph(c))
                {
                    token << c;
                    state = KState::CALLSIGN;
                }
                else
                {
                    callsign = token.str();
                    token.clear();
                    token.str("");
                    state = KState::WHITESPACE;
                }
                break;
            
            case KState::WHITESPACE:
                if (c == 0xC0)
                    finished = true;
                else if (isgraph(c))
                {
                    token << c;
                    state = KState::TOKEN;
                }
                else
                    state = KState::WHITESPACE;
                break;

            case KState::TOKEN:
                if (c == 0xC0)
                {
                    finished = true;
                    cmds.push_back(command_from_token(token.str()));
                    token.clear();
                    token.str("");
                }
                else if (isgraph(c))
                {
                    token << c;
                    state = KState::TOKEN;
                }
                else 
                {
                    cmds.push_back(command_from_token(token.str()));
                    token.clear();
                    token.str("");
                    state = KState::WHITESPACE;
                }
                break;
            
            default:
                finished = true;
        }
    }

    return cmds;
}

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
