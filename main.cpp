#include <unistd.h>
#include <termios.h>
#include <iostream>
#include <string>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "globals.h"
#include "normalmode.cpp"
#include "commandmode.cpp"

using namespace std;

/*******************************************
* Cleanup and exit on error
*******************************************/
void cleanupOnError(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

/*******************************************
* Get terminal max width and height and set
* statusbarline, normalmode startline etc.
*******************************************/
void getTerminalSize() {
  	struct winsize ws;
  	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
  		cleanupOnError("ioctl failed");
  	}
  	else{
    	max_y = ws.ws_col;
    	max_x = ws.ws_row;
    	max_w = max_x/2;
    	commandline = max_x;
    	messageline = commandline-1;
  	}
}

/*******************************************
* Signal handler for window resize
*******************************************/
void catch_ws_change(int signo) {
		getTerminalSize();
		refreshNormalModeDisplay();
}

/*******************************************
* Disable Canonical mode
*******************************************/
void disableNonCanonicalMode(){
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &default_termios);
}

/*******************************************
* Enable Canonical mode
*******************************************/
void enableNonCanonicalMode(){
	tcgetattr(STDIN_FILENO, &default_termios);
	atexit(disableNonCanonicalMode);

	struct termios current = default_termios;
	current.c_lflag = current.c_lflag & ~(ECHO | ICANON);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &current);
}

/*******************************************
* Get the absolute path of app root
*******************************************/
void getroot(){
	char cur[1024];
	if(getcwd(cur, sizeof(cur)) != NULL) {
		root = string(cur);
   	}
   	else{
       cleanupOnError("getcwd failed");
   }
}

/*******************************************
* Start the explorer with normal mode
*******************************************/
void startExplorer(){
	initNormalMode();
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
}

/*******************************************
* Application main
*******************************************/
int main(){
	enableNonCanonicalMode();
	getTerminalSize();
	getroot();
	signal(SIGWINCH, catch_ws_change);
	startExplorer();
	return 0;
}