#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <math.h>
#include <stack>
#include <iomanip>
#include <sys/wait.h>
#include "globals.h"
#include "common_utils.h"

using namespace std;

/*******************************************
* Normal mode globals
*******************************************/
int offset=0; // SCROLL OFFSET

int max_w; // MAX NORMAL MODE HEIGHT

int cursor_x=1;
int cursor_y=1;

int normalmodeline=1; // NORMAL MODE START LINE

/*******************************************
* Normal mode display buffer, stores
* filelist output
*******************************************/
class normalModeBuffer
{
	string buffer;
	int line_count;
public:
	// Init
	normalModeBuffer(){
		buffer = "";
		line_count = 0;
	}

	// Write a line
	void write(string s){
		buffer = buffer +"\033[2K" + s;
		line_count++;
	}

	// Reset buffer content
	void reset(){
		buffer = "";
		line_count = 0;
	}

	// Flush buffer onto the screen
	void displayframe(){
		cout << "\033[?25l";
		movecursor(normalmodeline,1);
		cout << buffer;
		for(int i=line_count;i<messageline;i++)
			cout << "\033[2K" << "\n";
		movecursor(cursor_x, cursor_y);
		cout << "\033[?25h";
		fflush(stdout);
	}
};

normalModeBuffer buffer;

/*******************************************
* Display info about a file 
*******************************************/
void displayfile(string filename ,struct stat filestat){
	char dr,ur,uw,ux,otr,otw,otx,gr,gw,gx;

	dr = S_ISREG(filestat.st_mode) ? '-':'d';
	ur = (filestat.st_mode & S_IRUSR) ? 'r':'-';
	uw = (filestat.st_mode & S_IWUSR) ? 'w':'-';
	ux = (filestat.st_mode & S_IXUSR) ? 'x':'-';
	otr = (filestat.st_mode & S_IROTH) ? 'r':'-';
	otw = (filestat.st_mode & S_IWOTH) ? 'w':'-';
	otx = (filestat.st_mode & S_IXOTH) ? 'x':'-';
	gr = (filestat.st_mode & S_IRGRP) ? 'r':'-';
	gw = (filestat.st_mode & S_IWGRP) ? 'w':'-';
	gx = (filestat.st_mode & S_IXGRP) ? 'x':'-';

	stringstream permission;
	permission << dr << ur << uw << ux << otr << otw << otx << gr << gw << gx;

	long long int filesize = filestat.st_size;

	string ret_size = convert_size(filesize);
	char* owner = getpwuid(filestat.st_uid)->pw_name;
	string ow = owner;
	char* group = getgrgid(filestat.st_gid)->gr_name;
	string gp = group;
	string time = ctime(&filestat.st_mtime);
	time.erase(time.find('\n',0),1);

	// DYNAMIC OUTPUT FORMATTING

	int dynamic_width = max_y-15-time.length();
	int ow_w = dynamic_width/5;
	int g_w = dynamic_width/5;

	if(ow.size()>ow_w){
		ow.resize(ow_w-2);
		ow += "..";
	}
	else{
		while(ow.size()<ow_w)
			ow += " ";	
	}
	if(gp.size()>g_w){
		gp.resize(g_w-2);
		gp += "..";
	}
	else{
		while(gp.size()<g_w)
			gp += " ";
	}

	while(ret_size.size()<5)
		ret_size += " ";

	string prefix = permission.str() + " " + ow + " " + gp + " " + time + " " + ret_size + " ";
	if(prefix.size()+filename.size()+1 > max_y){
		filename.resize(max_y-prefix.size()-3);
		filename += "..";
	}

	string result = prefix + " " + filename + "\n";
	buffer.write(result);
}

/*******************************************
* Get contents of a dir and display details
* of items
*******************************************/
void displayexplorer(){
	string current_path = forward_stack.top();
	vector<string> fl = listdir(current_path.c_str());
	cur_dir_size = fl.size();
	int n = fl.size();
	for(int i=0;i<min(max_w,n);i++){
		string fname = fl[offset+i];
		string file_path = current_path + fname;
		struct stat filestat;
		if(stat(file_path.c_str(),&filestat) == 0){
			displayfile(fname, filestat);
		}
	}
}

/*******************************************
* refresh normal mode screen content
*******************************************/
void refreshNormalModeDisplay(){
	buffer.reset();
	offset = 0;
	displayexplorer();
	buffer.displayframe();
	cursor_x = 1;
	cursor_y = 1;
	displayStatusBar();
}

/*******************************************
* Up arrow action 
*******************************************/
void up(){
	if(cursor_x>1){
		cursor_x--;
	}
	else{
		if(offset>0)
			offset--;
	}
}

/*******************************************
* Down arrow action
*******************************************/
void down(){
	if(cursor_x<max_w && cursor_x < cur_dir_size){
		cursor_x++;
	}
	else{
		if(offset+max_w < cur_dir_size)
			offset++;
	}
}

/*******************************************
* Enter directory or open file in vim
*******************************************/
void action(){
	string current_path = forward_stack.top();
	vector<string> fl = listdir(current_path.c_str());
	if(fl[offset+cursor_x-1] != "."){
		string file_path = current_path+fl[offset+cursor_x-1]+"/";
		struct stat filestat;
		if(stat(file_path.c_str(),&filestat) == 0){
			if(!S_ISREG(filestat.st_mode)){
				history.push(file_path);
				forward_stack.push(file_path);
				buffer.reset();
				offset = 0;
				displayexplorer();
				cursor_x = 1;
				cursor_y = 1;
			}
		}
		else{
			pid_t chpid = fork();
			file_path.pop_back();
			if(chpid == 0){
				execlp("/usr/bin/vi","vi",file_path.c_str(), NULL);
			}
			else{
				int return_s;
				waitpid(chpid, &return_s, 0);
			}
		}
	}
}

/*******************************************
* Move back on backspace
*******************************************/
void back(){
	if(history.size()>1){
		history.pop();
		forward_stack.push(history.top());
		buffer.reset();
		offset = 0;
		displayexplorer();
		cursor_x = 1;
		cursor_y = 1;
	}
}

/*******************************************
* Move front on right arrow
*******************************************/
void forward(){
	if(backward_stack.size()>0){
		forward_stack.push(backward_stack.top());
		backward_stack.pop();
		buffer.reset();
		offset = 0;
		displayexplorer();
		cursor_x = 1;
		cursor_y = 1;
	}
}

/*******************************************
* Move to previous directory on left arrow
*******************************************/
void backward(){
	if(forward_stack.size()>1){
		backward_stack.push(forward_stack.top());
		forward_stack.pop();
		buffer.reset();
		offset = 0;
		displayexplorer();
		cursor_x = 1;
		cursor_y = 1;
	}
}

/*******************************************
* Jump to application root
*******************************************/
void home(){
	if(!forward_stack.empty() && forward_stack.top() !="./"){
		forward_stack.push("./");
		buffer.reset();
		offset = 0;
		displayexplorer();
		cursor_x = 1;
		cursor_y = 1;
	}
}

/*******************************************
* Check if normal mode display is
* to be refreshed
*******************************************/
void checkAndupdateFrame(char action){
	if(action == 'd'){
		if(cursor_x == max_w){
			if(offset+max_w <= cur_dir_size){
				buffer.reset();
				displayexplorer();
			}
		}
	}
	else if(action == 'u'){
		if(cursor_x == 1 && offset>=0){
			buffer.reset();
			displayexplorer();
		}
	}
}

/*******************************************
* k press
*******************************************/
void prevpage(){
	cursor_x = 1;
	up();
	checkAndupdateFrame('u');
	cursor_x = 1;
}

/*******************************************
* l press
*******************************************/
void nextpage(){
	cursor_x = max_w;
	down();
	checkAndupdateFrame('d');
	cursor_x = 1;
}

/*******************************************
* Normal mode loop
*******************************************/
bool NormalMode(){
	char c;
	movecursor(1,1);
  	displayexplorer();
  	buffer.displayframe();

	while (c != 'q'){
		c = cin.get();
		switch(c){
			case 65: up(); checkAndupdateFrame('u'); break;
			case 66: down(); checkAndupdateFrame('d'); break;
			case 10: action(); break;
			case 127: back(); break;
			case 67: forward(); break;
			case 68: backward(); break;
			case 'l': nextpage();  break;
			case 'k': prevpage(); checkAndupdateFrame('u'); break;
			case 'h': home(); break;
			case ':': initCommandMode(); displayStatusBar(); break; 
		}
		buffer.displayframe();
	}
	write(STDOUT_FILENO, "\x1b[2J", 4);
	return false;
}

/*******************************************
* Init normal mode and enter 
* normal mode loop
*******************************************/
bool initNormalMode(){
	write(STDOUT_FILENO, "\x1b[2J", 4);
	buffer.reset();
	history.push("./");
	forward_stack.push("./");
	normal_mode = true;
	displayStatusBar();
	return NormalMode();
}
