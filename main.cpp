#include "myheader.h"

using namespace std;

void display_arr_on_terminal( int current_cursor_pos, vector <string> &arr);
string get_parent_directory(string path);
void open_file(string path);
void change_dir(string path);
bool search_command(string path, string filename);
bool is_safe_to_list(string path);
bool create_file_command(string path, string filename);

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

void initialise_terminal(){
  if( get_terminal_rows_and_cols(&E.number_of_rows_terminal, &E.number_of_cols_terminal) == -1)
    die("get_terminal_rows_and_cols");
  E.row_no = 0;
  E.window_size = 10;
  E.start_row=0;
  E.end_row=0;
//   E.current_path="/home/yash";
  change_dir(".");
}

/** command functionalities **/
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
}

bool create_file_command(string path, string filename){
    if(path!="/"){
        if(path[0]=='~'){
            string to_append = E.current_path;
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
            string to_append = E.current_path;
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


/** helper functions **/
void change_dir(string path){
    char abs_path[2000];
    realpath(path.c_str(), abs_path);
    // cout << "  abs: " << abs_path;
    chdir(abs_path);
    string current_dir(abs_path);
    E.current_path = current_dir;
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
    cout << "---------------NORMAL MODE--------------- : " << E.current_path << endl;
    gotoxy(0,0); 
}

void print_command_mode_at_end(){
    gotoxy(0, E.number_of_rows_terminal-3);
    cout << "---------------COMMAND MODE-------------- : " << E.current_path << endl;
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

bool is_safe_to_list(string path){
    if( dirExists(E.current_path.c_str()) && is_directory(E.current_path) )
        return true;
    else return false;
}

// void get_real_path(){
    
// }

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
                if(command == "goto"){
                    E.prev_stack.push(E.current_path);
                    change_dir(tokens[1]);
                    if(is_safe_to_list(E.current_path)){
                        E.row_no=0;
                        E.start_row=0;
                        E.end_row=0;
                        render_blank_screen();
                        reposition_cursor_to_start();
                        get_files(E.current_path.c_str());
                        print_command_mode_at_end();
                        gotoxy(0,E.number_of_rows_terminal-2);
                    } else {
                        string popped_path = E.prev_stack.top();
                        E.prev_stack.pop();
                        change_dir(popped_path);
                        render_blank_screen();
                        reposition_cursor_to_start();
                        get_files(E.current_path.c_str());
                        print_command_mode_at_end();
                        gotoxy(0,E.number_of_rows_terminal-1);
                        cout << "Invalid path or a file path!";
                        gotoxy(0,E.number_of_rows_terminal-2);
                    }
                } else if(command == "copy") {
                    cout << "copy command here";
                } else if(command == "move") {

                } else if(command == "rename") {

                } else if(command == "delete") {

                } else if(command == "create_file") {
                    string file_name = tokens[1];
                    string dest_path = tokens[2];
                    bool create_file_status = create_file_command(dest_path, file_name);
                    if(create_file_status){
                        render_blank_screen();
                        reposition_cursor_to_start();
                        get_files(E.current_path.c_str());
                        print_command_mode_at_end();
                        gotoxy(0,E.number_of_rows_terminal-1);
                        cout << "Created " << file_name << " successfully!";
                        gotoxy(0,E.number_of_rows_terminal-2);
                    } else {
                        render_blank_screen();
                        reposition_cursor_to_start();
                        get_files(E.current_path.c_str());
                        print_command_mode_at_end();
                        gotoxy(0,E.number_of_rows_terminal-1);
                        cout << "File not created! Enter correct destination path.";
                        gotoxy(0,E.number_of_rows_terminal-2);
                    } 
                } else if(command == "create_dir"){
                    string dir_name = tokens[1];
                    string dest_path = tokens[2];
                    bool create_dir_status = create_directory_command(dest_path, dir_name);
                    if(create_dir_status){
                        render_blank_screen();
                        reposition_cursor_to_start();
                        get_files(E.current_path.c_str());
                        print_command_mode_at_end();
                        gotoxy(0,E.number_of_rows_terminal-1);
                        cout << "Created " << dir_name << " successfully!";
                        gotoxy(0,E.number_of_rows_terminal-2);
                    } else {
                        render_blank_screen();
                        reposition_cursor_to_start();
                        get_files(E.current_path.c_str());
                        print_command_mode_at_end();
                        gotoxy(0,E.number_of_rows_terminal-1);
                        cout << "Directory not created! Enter correct destination path.";
                        gotoxy(0,E.number_of_rows_terminal-2);
                    }
                } else if(command == "search") {
                    string file_name = tokens[1];
                    bool is_found = search_command(E.current_path, file_name);
                    if(is_found){
                        render_blank_screen();
                        reposition_cursor_to_start();
                        get_files(E.current_path.c_str());
                        print_command_mode_at_end();
                        gotoxy(0,E.number_of_rows_terminal-1);
                        cout << file_name << " is available!";
                        gotoxy(0,E.number_of_rows_terminal-2);
                    } else {
                        render_blank_screen();
                        reposition_cursor_to_start();
                        get_files(E.current_path.c_str());
                        print_command_mode_at_end();
                        gotoxy(0,E.number_of_rows_terminal-1);
                        cout << "Not Available!";
                        gotoxy(0,E.number_of_rows_terminal-2);

                    }
                }
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


/**** Functionalities ****/
void home_backspace_left_right_common(){
        E.start_row=0;
        E.end_row=E.window_size-1;
        E.row_no=0;
        const char* the_path = E.current_path.c_str();
        get_files(the_path);
        print_normal_mode_at_end();
}

void on_press_home(){
    if( E.current_path!="/home/yash"){
        string home_path = "/home/yash";
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
    write(STDOUT_FILENO, "\x1b[2J", 4);
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

        cout << cursor << permissions << "\t\t" << username << "\t\t" << groupname << "\t\t" << modified_time << "\t\t" << size_of_file << "\t\t" << arr[i] << endl;        
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
            print_command_mode_at_end();
            handle_command_mode();
        }
    }
  return 0;
}