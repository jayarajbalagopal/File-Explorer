#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstring>
#include "globals.h"
#include "common_utils.h"

using namespace std;

/*******************************************
* Command mode command execution status
*******************************************/
void displayCmdStatus() {
    if(errno != 0){
        movecursor(messageline, 1);
        cout << "\033[2K";
        perror("ERROR");
    }
    else{
        refreshNormalModeDisplay();
        movecursor(messageline, 1);
        cout << "\033[2K";
        cout << "SUCCESS";
    }
    fflush(stdin);
}

/*******************************************
* Display a message at messageline
*******************************************/
void displayCmdStatus(const char* s){
    movecursor(messageline, 1);
    cout << "\033[2K";
    cout << s;
    fflush(stdout);
}

/*******************************************
* Copy a file
*******************************************/
void copy_file(string source_file, string destination_dir){
    char* path = new char[source_file.size()];
    strcpy(path, source_file.c_str());
    string base_name = basename(path);
    string destination_file =  destination_dir + "/" + base_name;
	int fd[2];
    fd[0] = open(source_file.c_str(), O_RDONLY);
    if (fd[0] == -1){
        return;
    }
    fd[1] = open(destination_file.c_str(), O_WRONLY | O_CREAT | S_IRUSR | S_IWUSR);
    if (fd[1] == -1){
        close(fd[0]);
       	return;
    }

    char stream_buffer[1024];
    int count = 0;

    while ((count = read(fd[0], stream_buffer, sizeof(stream_buffer))) != 0)
        write(fd[1], stream_buffer, count);

    struct stat source_info;
    stat(source_file.c_str(),&source_info);
    fchown(fd[1],source_info.st_uid,source_info.st_gid);
    fchmod(fd[1],source_info.st_mode);
}

/*******************************************
* Move a file
*******************************************/
void move_file(string source_file, string destination_dir){
    copy_file(source_file, destination_dir);
    remove(source_file.c_str());
}

/*******************************************
* Rename file or directory
*******************************************/
void rename_item(string source, string destination){
    rename(source.c_str(), destination.c_str());
}

/*******************************************
* Make a directory with 0777
*******************************************/
void make_dir(string dir_name, string destination_dir){
    string dest_path = destination_dir + "/" + dir_name;
    struct stat dir_info;
    if(stat(dest_path.c_str(), &dir_info) == -1) {
        mkdir(dest_path.c_str(), 0777);
        errno = 0;
    }
}

/*******************************************
* Make a file
*******************************************/
void make_file(string file_name, string destination_dir){
    string dest_path = destination_dir + "/" + file_name;
    int fd = open(dest_path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR);
}

/*******************************************
* Delete a file
*******************************************/
void delete_file(string source_file){
    remove(source_file.c_str());
}

/*******************************************
* Delete a directory
*******************************************/
void delete_dir(string dir_name){
    vector<string> contents = listdir(dir_name);
    for(int i=0;i<contents.size();i++){
        struct stat filestat;
        string path = dir_name+"/"+contents[i];
        if(stat(path.c_str(),&filestat) == 0){
            if(!S_ISREG(filestat.st_mode)){
                if(contents[i]!="." && contents[i]!=".."){
                    delete_dir(path);
                }
            }
            else{
                delete_file(path);
            }
        }
    }
    remove(dir_name.c_str());
}


/*******************************************
* Copy a directory
*******************************************/
void copy_dir(string dir_name, string destination_dir){
    char* path = new char[dir_name.size()];
    strcpy(path, dir_name.c_str());
    string base_dir_name = basename(path);
    
    vector<string> contents = listdir(dir_name);
    make_dir(base_dir_name, destination_dir);

    struct stat dir_info;
    stat(dir_name.c_str(),&dir_info);
    string target_dir = destination_dir+"/"+base_dir_name;
    chown(target_dir.c_str(),dir_info.st_uid,dir_info.st_gid);
    chmod(target_dir.c_str(),dir_info.st_mode);

    for(int i=0;i<contents.size();i++){
        struct stat filestat;
        string path = dir_name+"/"+contents[i];
        if(stat(path.c_str(),&filestat) == 0){
            if(!S_ISREG(filestat.st_mode)){
                if(contents[i]!="." && contents[i]!=".."){
                    copy_dir(path, destination_dir+"/"+base_dir_name);
                }
            }
            else{
                copy_file(path, destination_dir+"/"+base_dir_name);
            }
        }
    }
}

/*******************************************
* Move a directory
*******************************************/
void move_dir(string dir_name, string destination_dir){
    copy_dir(dir_name, destination_dir);
    delete_dir(dir_name);
}

/*******************************************
* Search for a file
*******************************************/
bool search(string dir_name, string filename){
    vector<string> contents = listdir(dir_name);
    vector<string> dirs;
    for(int i=0;i<contents.size();i++){
        struct stat filestat;
        string path = dir_name+"/"+contents[i];
        if(stat(path.c_str(),&filestat) == 0){
            if(!S_ISREG(filestat.st_mode)){
                if(contents[i]!="." && contents[i]!=".."){
                    dirs.push_back(path);
                }
            }
            else{
                if(contents[i] == filename)
                    return true;
            }
        }
    }
    for(auto dir: dirs){
        bool found = search(dir, filename);
        if(found)
            return true;
    }
    return false;
}

/*******************************************
* Copy wrapper
*******************************************/
void copy(vector<string> sources, string destination_dir){
    for(int i=0;i<sources.size();i++){
        struct stat filestat;
        if(stat(sources[i].c_str(),&filestat) == 0){
            if(!S_ISREG(filestat.st_mode)){
                if(sources[i]!="." && sources[i]!=".."){
                    copy_dir(sources[i], destination_dir);
                }
            }
            else{
                copy_file(sources[i], destination_dir);
            }
        }
    } 
}

/*******************************************
* Move wrapper
*******************************************/
void move(vector<string> sources, string destination_dir){
    for(int i=0;i<sources.size();i++){
        struct stat filestat;
        if(stat(sources[i].c_str(),&filestat) == 0){
            if(!S_ISREG(filestat.st_mode)){
                if(sources[i]!="." && sources[i]!=".."){
                    move_dir(sources[i], destination_dir);
                }
            }
            else{
                move_file(sources[i], destination_dir);
            }
        }
    } 
}

/*******************************************
* Goto directory
*******************************************/
void goto_dir(string destination){
    string path = destination;
    if(destination[destination.size()-1]!='/')
        path.push_back('/');
    struct stat dirstat;
    if(stat(path.c_str(),&dirstat) == 0){
        forward_stack.push(path);
    }
}

/*******************************************
* Generate relative or absolute path
* from input
*******************************************/
void formatpath(string& s){
    if(s[0] == '~'){
        s[0] = '/';
        s = '.'+s;
    }
    else if(s[0] != '/'){
        s = forward_stack.top()+s;
    }
}

/*******************************************
* Generate relative or absolute path
* from input
*******************************************/
void formatpath(vector<string>&s){
    for(int i=0;i<s.size();i++){
        if(s[i][0] == '~'){
            s[i][0] = '/';
            s[i] = '.'+s[i];
        }
        else if(s[i][0]!='/'){
            s[i] = forward_stack.top()+s[i];
        }
    }
}

/*******************************************
* Analyse tokens and execute command
*******************************************/
void executeCommand(vector<string> tokens){
    errno = 0;
    if(tokens.size()>0){
        if(tokens[0] == "copy"){
            if(tokens.size()>2){
                vector<string> sources;
                for(int i=1;i<tokens.size()-1;i++)
                    sources.push_back(tokens[i]);
                string destination_dir = tokens[tokens.size()-1];
                formatpath(sources);
                formatpath(destination_dir);
                copy(sources, destination_dir);
                displayCmdStatus();
            }
            else{
                displayCmdStatus("Atleast 2 Arguments expected for copy");
            }
        }
        else if(tokens[0] == "move"){
            if(tokens.size()>2){
                vector<string> sources;
                for(int i=1;i<tokens.size()-1;i++)
                    sources.push_back(tokens[i]);
                string destination_dir = tokens[tokens.size()-1];
                formatpath(sources);
                formatpath(destination_dir);
                move(sources, destination_dir);
                displayCmdStatus();
            }
            else{
                displayCmdStatus("Atleast 2 Arguments expected for move");
            }
        }
        else if(tokens[0] == "rename"){
            if(tokens.size() == 3){
                formatpath(tokens[1]);
                formatpath(tokens[2]);
                rename_item(tokens[1], tokens[2]);
                displayCmdStatus();
            }
            else{
                displayCmdStatus("Exactly 2 Arguments expected for rename");
            }
        }
        else if(tokens[0] == "create_dir"){
            if(tokens.size() == 3){
                formatpath(tokens[2]);
                make_dir(tokens[1], tokens[2]);
                displayCmdStatus();
            }
            else{
                displayCmdStatus("Exactly 2 Arguments expected for create_dir");
            }
        }
        else if(tokens[0] == "create_file"){
            if(tokens.size() == 3){
                formatpath(tokens[2]);
                make_file(tokens[1], tokens[2]);
                displayCmdStatus();
            }
            else{
                displayCmdStatus("Exactly 2 Arguments expected for create_file");
            }
        }
        else if(tokens[0] == "delete_file"){
            if(tokens.size() == 2){
                formatpath(tokens[1]);
                delete_file(tokens[1]);
                displayCmdStatus();
            }
            else{
                displayCmdStatus("Exactly 1 Argument expected for delete_file");
            }
        }
        else if(tokens[0] == "delete_dir"){
             if(tokens.size() == 2){
                formatpath(tokens[1]);
                string temp = tokens[1];
                delete_dir(tokens[1]);
                displayCmdStatus();
            }
            else{
                displayCmdStatus("Exactly 1 Argument expected for delete_dir");
            }
        }
        else if(tokens[0] == "goto"){
            if(tokens.size() == 2){
                formatpath(tokens[1]);
                goto_dir(tokens[1]);
                displayCmdStatus();
            }
            else{
                displayCmdStatus("Exactly 1 Argument expected for goto");
            }
        }
        else if(tokens[0] == "search"){
             if(tokens.size() == 2){
                bool found = search(forward_stack.top(), tokens[1]);
                if(found)
                    displayCmdStatus("FOUND");
                else
                    displayCmdStatus("NOT FOUND");
            }
            else{
                displayCmdStatus("Exactly 1 Arguments expected for search");
            }
        }
        else{
            displayCmdStatus("Invalid command");
        }
    }
}

/*******************************************
* Process input string and tokenize
*******************************************/
void processCommandString(string input){
    vector<string> tokens;
    string token = "";
    for(int i=0;i<input.size();i++){
        if(input[i]==' '){
            if(!token.empty())
                tokens.push_back(token);
            token = "";
        }
        else{
            token += input[i];
        }
    }
    if(token!="")
        tokens.push_back(token);
    executeCommand(tokens);
}

/*******************************************
* Trim spaces
*******************************************/
string trim(string input){
    string result;
    for(int i=0;i<input.size();i++)
        if(input[i]!=' ')
            result+=input[i];
    return result;
}

/*******************************************
* Command mode, Commandline input
*******************************************/
bool commandModeProcessor(){
    char c;
    string token;
    movecursor(commandline,1);
    cout << "\033[2K" << "Enter Command $ ";
    string command;
    while (true){
        c = cin.get();
        switch(c){
            case 10: processCommandString(command); return true;
            case 127: if(command.size()>0){ command.pop_back(); cout << "\b \b";} break;
            case 27: return false;
            default: if(!iscntrl(c)){command+=c; cout << c;} break;
        }
    }
    return false;
}

/*******************************************
* Init and loop commandmode
*******************************************/
bool initCommandMode(){
    normal_mode = false;
    while(commandModeProcessor());

    write(STDOUT_FILENO, "\x1b[2J", 4);
    normal_mode = true;
    return true;
}