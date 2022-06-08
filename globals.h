#ifndef GLOBALS
#define GLOBALS

#include <string>
#include <termios.h>
#include <stack>

// GLOBALS

int commandline;  // STATUS BAR
int messageline;  // COMMAND MODE STATUS

int max_x; // TERMINAL HEIGHT
int max_y; // TERMINAL WIDTH

std::string root; // ROOT DIR

struct termios default_termios; // DEFAULT TERMIOS

bool normal_mode; // MODE FLAG

int cur_dir_size; // CUR DIRECTORY SIZE
std::stack<std::string> history; // BUFFER STACK 
std::stack<std::string> forward_stack; // ACTION STACK
std::stack<std::string> backward_stack; // BUFFER STACK FOR LEFT ARROW

#endif
