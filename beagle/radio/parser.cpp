#include "parser.h"

using std::string;
using std::vector;
using std::stringstream;

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