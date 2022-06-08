#include <string>
#include <vector>
#include <dirent.h>
#include <iostream>
#include "globals.h"

using namespace std;

/*******************************************
* For listing contents of a directory
*******************************************/
vector<string> listdir(string dirname){
	DIR *d;
	struct dirent *dir;
	vector<string> filenames;

	d = opendir(dirname.c_str());
	if(d){
  	while ((dir = readdir(d)) != NULL) {
  		filenames.push_back(dir->d_name);
  	}
  	closedir(d);
	}
	return filenames;
}

/*******************************************
* Convert size from bytes
*******************************************/
string convert_size(long long int bytes){
	int count = 0;
	string mem_map[6] = {"B","KB","MB","GB","PB","TB"};
	while(bytes/1024>1){
		bytes/=1024;
		count++;
	}
	string result;
	result = to_string(bytes)+mem_map[count];
	return result;
}

/*******************************************
* Reset cursor to top left corner
*******************************************/
void resetcursor(){
	cout << "\033[;H";
}

/*******************************************
* move cursor to the given coordinates
*******************************************/
void movecursor(int x, int y){
    cout<<"\033["<<x<<";"<<y<<"H";
    fflush(stdout);
}

/*******************************************
* Display the status bar
*******************************************/
void displayStatusBar(){
	movecursor(commandline,1);
	if(normal_mode)
		cout << "\033[2K" << ":NORMAL MODE";
	else
		cout << "\033[2K" << "Enter Command $ ";
}