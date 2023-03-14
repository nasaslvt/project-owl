#include <string>
#include <vector>
#include <sstream>

// A1 B2 C3 D4 E5 F6 G7 H8
enum class Command {RIGHT_60, LEFT_60, TAKE_PICTURE, MODE_GRAYSCALE, MODE_COLOR, PICTURE_180, SPECIAL_FILTER, REMOVE_FILTER, UNDEFINED};
enum class KState {NO_FRAME, FRAME_START, WHITESPACE_INITIAL, CALLSIGN, WHITESPACE, TOKEN};

std::string to_string(Command cmd);
Command command_from_token(std::string token);
std::string token_from_command(Command cmd);
std::vector<Command> parse_buffer(char *buffer, int n, std::string &callsign);
