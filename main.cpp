#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <sys/ioctl.h>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <stack>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <signal.h>

using namespace std;

void reposition_cursor_to_start();
void print_command_mode_at_end();
void print_normal_mode_at_end();
int get_files(const char* pathname);
void gotoxy(int x, int y);
int fileExists(const char *path);
int get_terminal_rows_and_cols(int *rows, int *cols);
void initialise_terminal();
void die(const char *s);
void enable_canonical_mode();
void enable_non_canonical_mode();
void render_blank_screen(); 
void display_arr_on_terminal( int current_cursor_pos, vector <string> &arr);
string get_parent_directory(string path);
void open_file(string path);
void change_dir(string path);
bool search_command(string path, string filename);
bool is_safe_to_list(string path);
bool create_file_command(string path, string filename);
string get_tilda_dir();
bool move_command(string old_path, string new_path);
string get_last_child(string path);
bool copydir(string source_dir, string dest_dir);
bool copyfile_command(string source_file_path, string dest_dir_path);
bool delete_file_command(string path);
bool delete_dir_command(string source_dir_path);
void resize_handler(int trigger);

vector<string> record_keeper;

/**** Initial terminal attributes and related functions ****/
struct terminal_configs {
    int row_no;
    int number_of_rows_terminal;
    int number_of_cols_terminal;
    int start_row;
    int end_row;
    int window_size;
    string current_path;
    struct termios orig_termios;
    stack<string> prev_stack;
    stack<string> next_stack;
    int is_normal_mode;
};
struct terminal_configs E;

int get_terminal_rows_and_cols(int *rows, int *cols){
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
      return -1;
    } else {
      *cols = ws.ws_col;
      *rows = ws.ws_row;
      return 0;
    }
}

void resize_handler(int trigger){
    if( get_terminal_rows_and_cols(&E.number_of_rows_terminal, &E.number_of_cols_terminal) == -1)
        die("get_terminal_rows_and_cols");
    // render_blank_screen();
    get_files(E.current_path.c_str());
    if(E.is_normal_mode) print_normal_mode_at_end();
    else print_command_mode_at_end();
}

void initialise_terminal(){
  if( get_terminal_rows_and_cols(&E.number_of_rows_terminal, &E.number_of_cols_terminal) == -1)
    die("get_terminal_rows_and_cols");
  E.row_no = 0;
  E.window_size = E.number_of_rows_terminal-5;
  E.start_row=0;
  E.end_row=0;
  change_dir(".");
  E.is_normal_mode=1;
}

/** command functionalities **/
bool copydir(string source_dir, string dest_dir){

    if(source_dir!="/"){
        if(source_dir[0]=='~'){
            string to_append = get_tilda_dir();
            source_dir = to_append + source_dir.substr(1, source_dir.length()-1);
        }
        char abs_path[2000];
        realpath(source_dir.c_str(), abs_path);
        string temp(abs_path);
        source_dir = temp;
        if(!is_safe_to_list(source_dir)) return false;
    }

    if(dest_dir!="/"){
        if(dest_dir[0]=='~'){
            string to_append = get_tilda_dir();
            dest_dir = to_append + dest_dir.substr(1, dest_dir.length()-1);
        }
        char abs_path[2000];
        realpath(dest_dir.c_str(), abs_path);
        string temp(abs_path);
        dest_dir = temp;
        if(!is_safe_to_list(dest_dir)) return false;
    }

	DIR * dir;
	struct dirent * d;
	struct stat info;
	dir = opendir(source_dir.c_str());
	if (dir == NULL) return false;
	while ((d = readdir(dir)))
	{
		string sname = source_dir + "/" + string(d->d_name);
		string dname = dest_dir + "/" + string(d->d_name);
		stat(sname.c_str(), &info);
		string cname = string(d->d_name);
		if (cname == "." || cname == "..")
			continue;
		else if (S_ISDIR(info.st_mode)){		
		    mkdir(dname.c_str(),0777);
			copydir(sname, dname);
		}
		else{
            string temp_dname = get_parent_directory(dname);
			copyfile_command(sname, temp_dname);
        }
			
	}
	closedir(dir);
    return true;
}

bool copyfile_command(string source_file_path, string dest_dir_path){
    if(source_file_path!="/"){
        if(source_file_path[0]=='~'){
            string to_append = get_tilda_dir();
            source_file_path = to_append + source_file_path.substr(1, source_file_path.length()-1);
        }
        char abs_path[2000];
        realpath(source_file_path.c_str(), abs_path);
        string temp(abs_path);
        source_file_path = temp;
        if(!fileExists) return false;
    }
    cout << source_file_path << endl;

    if(dest_dir_path!="/"){
        if(dest_dir_path[0]=='~'){
            string to_append = get_tilda_dir();
            dest_dir_path = to_append + dest_dir_path.substr(1, dest_dir_path.length()-1);
        }
        char abs_path[2000];
        realpath(dest_dir_path.c_str(), abs_path);
        string temp(abs_path);
        dest_dir_path = temp;
        if(!is_safe_to_list(dest_dir_path)) return false;
    }
    cout << dest_dir_path << endl;
    dest_dir_path = dest_dir_path + "/" + get_last_child(source_file_path);
	FILE *copy_from, *write_to;
	copy_from = fopen(source_file_path.c_str(), "r");
	write_to = fopen(dest_dir_path.c_str(), "w");
	if (copy_from == NULL) {
		return false;
    }
    if( write_to == NULL){
		return false;
    }
	char input;
	while ((input = getc(copy_from)) != EOF)
		putc(input, write_to);
	struct stat source_temp;
	stat(source_file_path.c_str(), &source_temp);
	chown(dest_dir_path.c_str(), source_temp.st_uid, source_temp.st_gid);
	chmod(dest_dir_path.c_str(), source_temp.st_mode);
	fclose(write_to);
	fclose(copy_from);
	return true;
}

bool search_command(string path, string filename)
{
	DIR * dir;
	struct dirent * cur_dir;
	struct stat file_info;
	dir = opendir(path.c_str());
	if (dir == NULL)
		return false;
	while ((cur_dir = readdir(dir)))
	{
		stat(cur_dir->d_name, &file_info);
		string extracted_name = string(cur_dir->d_name);
		if (extracted_name == filename)
			return true;
		if (extracted_name == "." || extracted_name == "..")
			continue;
		else if (S_ISDIR(file_info.st_mode))
		{
		    string next = path + "/" + extracted_name;
			bool res = search_command(next, filename);
			if (res == true) return true;
		}
	}
	closedir(dir);
	return false;

    // vector<string> file_names;

}

bool move_command(string old_path, string new_path){
    if(old_path!="/"){
        if(old_path[0]=='~'){
            string to_append = get_tilda_dir();
            old_path = to_append + old_path.substr(1, old_path.length()-1);
        }
        char abs_path[2000];
        realpath(old_path.c_str(), abs_path);
        string temp(abs_path);
        old_path = temp;
    }

    if(new_path!="/"){
        if(new_path[0]=='~'){
            string to_append = get_tilda_dir();
            new_path = to_append + new_path.substr(1, new_path.length()-1);
        }
        char abs_path[2000];
        realpath(new_path.c_str(), abs_path);
        string temp(abs_path);
        new_path = temp;
        if(!is_safe_to_list(new_path)) return false;
    }

    // cout << old_path << " " << new_path << endl;
    string child = get_last_child(old_path);
    if(child=="") return false;
    new_path = new_path + "/" + child;
    int status = rename(old_path.c_str(), new_path.c_str());
    if( status==-1 ) return false;
    else return true; 
}


bool create_file_command(string path, string filename){
    if(path!="/"){
        if(path[0]=='~'){
            string to_append = get_tilda_dir();
            path = to_append + path.substr(1, path.length()-1);
        }
        char abs_path[2000];
        realpath(path.c_str(), abs_path);
        if(!is_safe_to_list(abs_path)) return false;
        string temp(abs_path);
        path = temp;
        path = path + "/" + filename;
    } 
    else path = path + filename;
    char abs_path[2000];
    realpath(path.c_str(), abs_path);
    const int new_file_status = creat(abs_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(new_file_status==-1) return false;
    else return true;
}

bool create_directory_command(string path, string dirname){
    if(path!="/"){
        if(path[0]=='~'){
            string to_append = get_tilda_dir();
            path = to_append + path.substr(1, path.length()-1);
        }
        char abs_path[2000];
        realpath(path.c_str(), abs_path);
        if(!is_safe_to_list(abs_path)) return false;
        string temp(abs_path);
        path = temp;
        path = path + "/" + dirname;
    } 
    else path = path + dirname;
    char abs_path[2000];
    realpath(path.c_str(), abs_path);
    const int new_dir_status = mkdir(abs_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (new_dir_status==-1) return false;
    else return true; 
}


bool rename_command(string old_path, string filename){
    if(old_path!="/"){
        if(old_path[0]=='~'){
            string to_append = get_tilda_dir();
            old_path = to_append + old_path.substr(1, old_path.length()-1);
        }
        char abs_path[2000];
        realpath(old_path.c_str(), abs_path);
        string temp(abs_path);
        old_path = temp;
    }
    string parent = get_parent_directory(old_path);
    if(!is_safe_to_list(parent)) return false;
    string new_path = parent + "/" + filename;

    int status = rename(old_path.c_str(), new_path.c_str());
    if( status==-1 ) return false;
    else return true; 
}

bool delete_file_command(string path){
    if(path!="/"){
        if(path[0]=='~'){
            string to_append = get_tilda_dir();
            path = to_append + path.substr(1, path.length()-1);
        }
        char abs_path[2000];
        realpath(path.c_str(), abs_path);
        string temp(abs_path);
        path = temp;
    }
    if(!fileExists(path.c_str())) return false;

    int delete_file_status = remove(path.c_str());
    if( delete_file_status==0 ) return true;
    else return false;
}

bool delete_dir_command(string source_dir_path){
    //getting real path
    if(source_dir_path!="/"){
        if(source_dir_path[0]=='~'){
            string to_append = get_tilda_dir();
            source_dir_path = to_append + source_dir_path.substr(1, source_dir_path.length()-1);
        }
        char abs_path[2000];
        realpath(source_dir_path.c_str(), abs_path);
        string temp(abs_path);
        source_dir_path = temp;
    }
    //getting real path

    if(fileExists(source_dir_path.c_str())) return false;

	DIR * dir;
	struct dirent * content;
	dir = opendir(source_dir_path.c_str());
	if (dir == NULL) return false;
	struct stat info;
	while ((content = readdir(dir))) {
		string new_source = source_dir_path + "/" + string(content->d_name);
		stat(new_source.c_str(), &info);
		string cname = string(content->d_name);
		if (cname == "." || cname == "..")
			continue;
		else if (S_ISDIR(info.st_mode)) delete_dir_command(new_source);
		else delete_file_command(new_source);
	}
	closedir(dir);
	remove(source_dir_path.c_str());
    return true;
}

/************************************* helper functions *****************************************/
void command_mode_print_status(string message){
    render_blank_screen();
    reposition_cursor_to_start();
    get_files(E.current_path.c_str());
    print_command_mode_at_end();
    gotoxy(0,E.number_of_rows_terminal-1);
    cout << message;
    gotoxy(0,E.number_of_rows_terminal-2);
}

void change_dir(string path){
    char abs_path[2000];
    realpath(path.c_str(), abs_path);
    // cout << "  abs: " << abs_path;
    chdir(abs_path);
    string current_dir(abs_path);
    E.current_path = current_dir;
}

string get_tilda_dir(){
    uid_t uid;
    passwd* pw;
    uid = geteuid();
    pw = getpwuid(uid);
    string dir(pw->pw_name);
    dir = "/home/" + dir;
    return dir; 
}

string get_last_child(string path){
    int p2 = path.length();
    int i;
    for(i=p2-1; path[i]!='/'; --i);
    string child = path.substr(i+1, path.length()-i);
    if(i==0) child="";
    return child;
}

bool is_directory(string path){
    struct stat file_data;
    const char* temp_path = path.c_str();
    stat(temp_path, &file_data);
    string is_dir="";
    is_dir += ((S_ISDIR(file_data.st_mode))  ? "d" : "-");
    if( is_dir=="-" ) return false;
    return true;
}

string get_parent_directory(string path){
    int p2 = path.length();
    int i;
    for(i=p2-1; path[i]!='/'; --i);
    string parent = path.substr(0, i);
    if(i==0) parent = "/"; 
    return parent; 
}

void open_file(string path){
    pid_t child=fork();
        if(child==0){
            string open_path_name;
            if(E.current_path=="/") open_path_name = "xdg-open " + E.current_path + "\'" + path + "\'"; 
            else open_path_name = "xdg-open " + E.current_path + "/" + "\'" + path + "\'";
            execl("/bin/sh", "sh", "-c", open_path_name.c_str() , (char *) NULL);
        }
}

void print_normal_mode_at_end(){
    gotoxy(0, E.number_of_rows_terminal-3);
    string to_echo = "-------------NORMAL MODE------------ : " + E.current_path; 
    cout << to_echo.substr(0, E.number_of_cols_terminal-1 ) << endl;
    gotoxy(0,0); 
}

void print_command_mode_at_end(){
    gotoxy(0, E.number_of_rows_terminal-3);
    string to_echo = "-------------COMMAND MODE------------ :" + E.current_path; 
    cout << to_echo.substr(0, E.number_of_cols_terminal-1) << endl;
    gotoxy(0,0); 
}

int dirExists(const char *path)
{
    struct stat info;

    if(stat( path, &info ) != 0)
        return 0;
    else if(info.st_mode & S_IFDIR)
        return 1;
    else
        return 0;
}

int fileExists(const char *path)
{
    struct stat info;

    if(stat( path, &info ) != 0)
        return 0;
    else if(info.st_mode & S_IFDIR)
        return 0;
    else
        return 1;
}

bool is_safe_to_list(string path){
    if( dirExists(path.c_str()) && is_directory(path) )
        return true;
    else return false;
}

vector<string> tokenise_string(string str, char delim){
    vector<string> tokens;
    string temp = "";
    for(int i = 0; i < str.length(); i++){
        if(str[i] == delim){
            tokens.push_back(temp);
            temp = "";
        }
        else
            temp += str[i];           
    }
    tokens.push_back(temp);
    return tokens;
}

void handle_command_mode(){
    gotoxy(0,E.number_of_rows_terminal-2);
    char c2;
    string to_echo="";
    while(1){
        c2 = cin.get();
        if(c2==27){
            E.is_normal_mode = 1;
            render_blank_screen();
            reposition_cursor_to_start();
            const char * the_path = E.current_path.c_str();
            get_files(the_path);
            print_normal_mode_at_end();
            break;
        } else if(to_echo=="quit"){
            render_blank_screen();
            reposition_cursor_to_start();
            exit(1);
        } else {
            if(c2!=10){
                if( c2!=127) to_echo += c2;
                else{
                    if(to_echo != "") 
                        if(to_echo.length()>1) to_echo=to_echo.substr(0, to_echo.length()-1);
                        else to_echo="";
                        render_blank_screen();
                        gotoxy(0,0);
                        const char * the_path = E.current_path.c_str();
                        get_files(the_path);
                        print_command_mode_at_end();  
                }
                gotoxy(0,E.number_of_rows_terminal-2);
                cout << to_echo;
            } else {
                //exec details here
                vector<string> tokens = tokenise_string(to_echo, ' ');
                to_echo="";
                string command = tokens[0];
                /******************** GOTO ****************************/
                if(command == "goto"){
                    if(tokens.size()!=2)
                        command_mode_print_status("Enter valid argument count!");
                    else {
                        string path = tokens[1];
                        if(path!="/"){
                            if(path[0]=='~'){
                                string to_append = get_tilda_dir();
                                path = to_append + path.substr(1, path.length()-1);
                            }
                            char abs_path[2000];
                            realpath(path.c_str(), abs_path);
                            string temp(abs_path);
                            path = temp;
                        }
                        E.prev_stack.push(E.current_path);
                        change_dir(path);
                        if(is_safe_to_list(E.current_path)){
                            E.row_no=0;
                            E.start_row=0;
                            E.end_row=0;
                            command_mode_print_status("");
                        } else {
                            string popped_path = E.prev_stack.top();
                            E.prev_stack.pop();
                            change_dir(popped_path);
                            command_mode_print_status("Invalid path or a file path!");
                        }
                    }
                    
                /******************** COPY FILE/DIRECTORY ****************************/
                } else if(command == "copy") {
                    if(tokens.size()<3)
                        command_mode_print_status("Enter valid argument count!");
                    else {
                        int files_count = tokens.size()-2;
                        int flag=0;
                        string dest_path = tokens.at(tokens.size()-1);
                        for( int i=1; i<=files_count; ++i){
                            string file_name = tokens[i];
                            //real path
                            if(file_name!="/"){
                                if(file_name[0]=='~'){
                                    string to_append = get_tilda_dir();
                                    file_name = to_append + file_name.substr(1, file_name.length()-1);
                                }
                                char abs_path[2000];
                                realpath(file_name.c_str(), abs_path);
                                string temp(abs_path);
                                file_name = temp;
                            }
                            //realpath
                            if(!dirExists(file_name.c_str())){
                                if(!copyfile_command(file_name, dest_path)){
                                    flag=1;
                                    break;
                                }
                            } else {
                                //realpath
                                if(dest_path!="/"){
                                    if(dest_path[0]=='~'){
                                        string to_append = get_tilda_dir();
                                        dest_path = to_append + dest_path.substr(1, dest_path.length()-1);
                                    }
                                    char abs_path[2000];
                                    realpath(dest_path.c_str(), abs_path);
                                    string temp(abs_path);
                                    dest_path = temp;
                                }
                                //realpath
                                string child = get_last_child(file_name);
                                create_directory_command(dest_path, child);
                                dest_path = dest_path + "/" + child;
                                if(!copydir(file_name, dest_path)){
                                    flag=1;
                                    break;
                                }
                            }
                        }
                        if(flag==0)
                            command_mode_print_status("All files/directories copied successfully!");
                        else
                            command_mode_print_status("Copy unsuccessful! Invalid file name or destination path.");
                    }
                }

                /******************** MOVE FILE/DIRECTORY ****************************/
                 else if(command == "move") {
                    if(tokens.size()<3)
                        command_mode_print_status("Enter valid argument count!");
                    else {
                        int files_count = tokens.size()-2;
                        int flag=0;
                        string dest_path = tokens.at(tokens.size()-1);
                        for( int i=1; i<=files_count; ++i){
                            string file_name = tokens[i];
                            if(!move_command(file_name, dest_path)){
                                flag=1;
                                break;
                            }
                        }
                        if(flag==0)
                            command_mode_print_status("All files/directories moved successfully!");
                        else
                            command_mode_print_status("Move unsuccessful! Invalid or root path.");
                    }
                }

                /******************** RENAME ****************************/
                else if(command == "rename") {
                    if(tokens.size()!=3) command_mode_print_status("Enter valid argument count!");
                    else {
                        string path = tokens[1];
                        string  new_name = tokens[2];
                        bool rename_status = rename_command(path, new_name);
                        if(rename_status)
                            command_mode_print_status("Renamed successfully");
                        else
                            command_mode_print_status("Rename not successful! Path is invalid or name already exists.");
                    }
                }
                
                /******************** DELETE FILE ****************************/
                else if(command == "delete_file") {
                    if(tokens.size()!=2) command_mode_print_status("Enter valid argument count!");
                    else {
                        string path = tokens[1];
                        bool delete_file_status = delete_file_command(path);
                        if(delete_file_status)
                            command_mode_print_status("Deleted file successfully!");
                        else command_mode_print_status("Operation unsuccessful! Enter correct path.");
                    }
                }
                
                /******************** DELETE DIRECTORY ****************************/
                else if(command=="delete_dir") {
                    if(tokens.size()!=2) command_mode_print_status("Enter valid argument count!");
                    else {
                        string path = tokens[1];
                        bool delete_dir_status = delete_dir_command(path);
                        if(delete_dir_status)
                            command_mode_print_status("Deleted directory successfully!");
                        else command_mode_print_status("Operation unsuccessful! Enter correct path.");
                    } 
                } 

                /******************** CREATE FILE ****************************/
                else if(command == "create_file") {
                    if(tokens.size()!=3) command_mode_print_status("Enter valid argument count!");
                    else {
                        string file_name = tokens[1];
                        string dest_path = tokens[2];
                        bool create_file_status = create_file_command(dest_path, file_name);
                        if(create_file_status){
                            string status_message = "Created " + file_name + " successfully!";
                            command_mode_print_status(status_message);
                        } else
                            command_mode_print_status("File not created! Enter correct destination path.");
                    }
                }
                /******************** CREATE DIRECTORY ****************************/
                else if(command == "create_dir"){
                    if(tokens.size()!=3) command_mode_print_status("Enter valid argument count!");
                    else {
                        string dir_name = tokens[1];
                        string dest_path = tokens[2];
                        bool create_dir_status = create_directory_command(dest_path, dir_name);
                        if(create_dir_status){
                            string status_message = "Created "+dir_name+" successfully!";
                            command_mode_print_status(status_message);
                        } else command_mode_print_status("Directory not created! Enter correct destination path.");
                    }
                }

                /******************** SEARCH ****************************/
                else if(command == "search") {
                    if(tokens.size()!=2) command_mode_print_status("Enter valid argument count!");
                    else {
                        string file_name = tokens[1];
                        bool is_found = search_command(E.current_path, file_name);
                        if(is_found){
                            string status_message = file_name + " is available";
                            command_mode_print_status(status_message);
                        } else command_mode_print_status("Search unsuccessful!");
                    }
                }

                /******************** WRONG COMMAND ****************************/
                else command_mode_print_status("Enter correct command!");
            }
        }
    }
}

/**** Error handling funtion ****/
void die(const char *s) {
    // render_blank_screen();
    // reposition_cursor_to_start();
    perror(s);
    exit(1);
}


/**** Normal Functionalities ****/
void home_backspace_left_right_common(){
        E.start_row=0;
        E.end_row=E.window_size-1;
        E.row_no=0;
        const char* the_path = E.current_path.c_str();
        get_files(the_path);
        print_normal_mode_at_end();
}

void on_press_home(){
    string home_p = get_tilda_dir();
    if( E.current_path!=home_p){
        string home_path = home_p;
        E.prev_stack.push(E.current_path);
        change_dir(home_path);
        home_backspace_left_right_common();
    }
}

void on_press_backspace(){
    if(E.current_path!="/"){
        string parent = get_parent_directory(E.current_path);
        E.prev_stack.push(E.current_path);
        change_dir(parent);
        home_backspace_left_right_common();
    }
}

void up_and_down_common(){
    display_arr_on_terminal(E.row_no, record_keeper);
    reposition_cursor_to_start();
    print_normal_mode_at_end();
}

void on_press_right(){
    if( !E.next_stack.empty() ){
        E.prev_stack.push(E.current_path);
        change_dir(E.next_stack.top());
        E.next_stack.pop();
        home_backspace_left_right_common();
    }
}

void on_press_left(){
    if( !E.prev_stack.empty() ){
        E.next_stack.push(E.current_path);
        change_dir(E.prev_stack.top());
        E.prev_stack.pop();
        home_backspace_left_right_common();
    }
}

/**** Modes ****/
void enable_canonical_mode() {
  if ( tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios)==-1 ) 
      die("tcsetattr");
}

void enable_non_canonical_mode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios)==-1)
        die("tcsgetattr");
    atexit(enable_canonical_mode);
    struct termios raw = E.orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1) die("tcssetattr");
}


/**** cursor related ****/
void reposition_cursor_to_start(){
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void gotoxy(int x, int y){
    printf("%c[%d;%df",0x1B,y,x);
}


/**** output screen related ****/
void render_blank_screen() {    
    cout << "\033[H\033[2J\033[3J";
}

int get_files(const char* pathname){
    record_keeper.clear();
    E.start_row=0;
    E.end_row=0;
    DIR* dir = opendir(pathname);
    if( dir==NULL ){
        render_blank_screen();
        cout << "No such directory!" << endl;
        return 1;
    }

    struct dirent* entity;
    entity = readdir(dir);
    while(entity!=NULL){
        record_keeper.push_back(entity->d_name);
        entity = readdir(dir);
    }
    sort(record_keeper.begin(), record_keeper.end());

    display_arr_on_terminal(E.row_no, record_keeper);
    reposition_cursor_to_start();
    closedir(dir);
    return 0;
}

void display_arr_on_terminal(int current_cursor_pos, vector<string> &arr){
    render_blank_screen();
    string cursor = "       ";
    
    if(E.row_no>(E.end_row-1)){
        E.start_row = E.end_row;
        E.end_row = arr.size()>(E.end_row+E.window_size) ? (E.end_row+E.window_size) : arr.size();
    } else {
        if(E.row_no<E.start_row && E.row_no!=0 && E.start_row!=0){
            E.end_row = E.start_row;
            E.start_row = (E.start_row-E.window_size)>0 ? (E.start_row-E.window_size) : 0;
        }
    }
    for( int i=E.start_row; i<E.end_row; ++i ){
        if( i==current_cursor_pos ) cursor=">>>  "; 
        else cursor = "     ";
        struct stat file_data;
        string temp = E.current_path;
        if(i==0) temp=E.current_path;
        else if(i==1){
            if( !E.prev_stack.empty()) temp=E.prev_stack.top();
            else temp=E.current_path;
        } 
        else temp = temp + "/" + record_keeper[i];
        
        const char* temp_path = temp.c_str();
        stat(temp_path, &file_data);  

        /*** modified time ***/
        string modified_time = ctime(&file_data.st_mtime);
        modified_time = modified_time.substr(4, 20);
        /*** filesize ***/
        string size_of_file="";
        int file_size = file_data.st_size;
        if( file_size>=1024 ){
            file_size = file_size/1024;
            if( file_size>=1024 ){
                file_size = file_size/1024;
                size_of_file = to_string(file_size);
                size_of_file = size_of_file + "GB";
            } else {
                size_of_file = to_string(file_size);
                size_of_file = size_of_file + "KB";
            }
        } else if(file_size>0) {
            size_of_file = to_string(file_size);
            size_of_file = size_of_file + "B";
        } else {
            size_of_file = to_string(0);
            size_of_file = size_of_file + "B";
        }

        /*** permissions ***/
        string permissions="";
        permissions += ((S_ISDIR(file_data.st_mode))  ? "d" : "-");
        permissions += ((file_data.st_mode & S_IRUSR) ? "r" : "-");
        permissions += ((file_data.st_mode & S_IWUSR) ? "w" : "-");
        permissions += ((file_data.st_mode & S_IXUSR) ? "x" : "-");
        permissions += ((file_data.st_mode & S_IRGRP) ? "r" : "-");
        permissions += ((file_data.st_mode & S_IWGRP) ? "w" : "-");
        permissions += ((file_data.st_mode & S_IXGRP) ? "x" : "-");
        permissions += ((file_data.st_mode & S_IROTH) ? "r" : "-");
        permissions += ((file_data.st_mode & S_IWOTH) ? "w" : "-");
        permissions += ((file_data.st_mode & S_IXOTH) ? "x" : "-");

        /*** User and group name ***/
        uid_t user_id = file_data.st_uid;
        uid_t group_id = file_data.st_gid;
        string username = (getpwuid(user_id)->pw_name);
        string groupname = (getgrgid(group_id)->gr_name);
        string blank = "    ";
        string to_echo = cursor + permissions + blank + username + blank + groupname + blank + modified_time + blank + size_of_file + blank + arr[i]; 
        to_echo = to_echo.substr(0, E.number_of_cols_terminal-2);
        cout << to_echo << endl;        
    }
}


int main() {
    enable_non_canonical_mode();
    render_blank_screen();
    initialise_terminal();
    reposition_cursor_to_start();
    const char * the_path = E.current_path.c_str();
    get_files(the_path);
    print_normal_mode_at_end();
    char c;
    while(1){
        E.is_normal_mode=1;
        signal(SIGWINCH, resize_handler);
        c = cin.get();
        if( c==10 /*enter key*/ ){
            string sub_path = record_keeper[E.row_no];
            E.prev_stack.push(E.current_path);
            string to_check_status_dir = E.current_path=="/" ? (E.current_path+sub_path): (E.current_path + "/" + sub_path);
            if(is_directory(to_check_status_dir)){
                change_dir(to_check_status_dir);
                E.start_row=0;
                E.end_row=E.window_size-1;
                E.row_no=0;
                the_path = E.current_path.c_str();
                get_files(the_path);
                print_normal_mode_at_end();
            } else {
                open_file(sub_path);
                print_normal_mode_at_end();
            }
        } else if( c=='h' ){
            on_press_home();
        } else if( c==127 /*backspace*/ ){
            on_press_backspace();
        } 
        else if( c=='q' ) {
            render_blank_screen();
            reposition_cursor_to_start();
            break;
        } else if (c == 'A' /*up*/ ) {
            if(E.row_no>0) E.row_no--;
            up_and_down_common();
        } else if(c == 'B' /*down*/){
            if( E.row_no < record_keeper.size()-1 ) E.row_no++;
            up_and_down_common();
        } else if(c == 'C' /*right*/){
            on_press_right();
        } else if(c == 'D' /*left*/){
            on_press_left();
        } else if( c == ':' ){
            E.is_normal_mode = 0;
            print_command_mode_at_end();
            handle_command_mode();
        }
    }
  return 0;
}