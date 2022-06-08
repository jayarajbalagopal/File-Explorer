#ifndef HEADER_FILE
#define HEADER_FILE

#include <vector>
#include <string>
#include "common_utils.cpp"

vector<string> listdir(string dirname);
string convert_size(long long int bytes);
void resetcursor();
void movecursor();
void displayStatusBar();
bool initCommandMode();
bool initNormalMode();
void refreshNormalModeDisplay();

#endif
