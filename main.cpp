#include "myheader.h"

using namespace std;

void display_arr_on_terminal( int current_cursor_pos, vector <string> &arr);
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
  E.window_size = E.number_of_rows_terminal-10;
  E.start_row=0;
  E.end_row=E.window_size-1;
  E.current_path="/home/yash";
  E.prev_stack.push("/");
  E.prev_stack.push("/home");
}


/**** Error handling funtion ****/
void die(const char *s) {
    // render_blank_screen();
    // reposition_cursor_to_start();
    perror(s);
    exit(1);
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
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1) die("tcssetattr");
}


/**** cursor related ****/
void reposition_cursor_to_start(){
    write(STDOUT_FILENO, "\x1b[H", 3);
}


/**** output screen related ****/
void render_blank_screen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
}

int get_files(const char* pathname){
    record_keeper.clear();
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
    for( int i=0; i<arr.size(); ++i ){
        if( i==current_cursor_pos ) cursor=">>>  "; 
        else cursor = "     ";

        struct stat file_data;
        string temp = E.current_path;
        if(i==0) temp=E.current_path;
        else if(i==1) temp=E.prev_stack.top();
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
        } else {
            size_of_file = to_string(file_size);
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
    initialise_terminal();
    render_blank_screen();
    reposition_cursor_to_start();
    const char * the_path = E.current_path.c_str();
    get_files(the_path);
    char c;
    while(1){
        c = cin.get();
        if( c=='q' ){
            render_blank_screen();
            reposition_cursor_to_start();
            break;
        } else if (c == 'A' /*up*/ ) {
            if(E.row_no>0) E.row_no--;
            the_path = E.current_path.c_str();
            display_arr_on_terminal(E.row_no, record_keeper);
            reposition_cursor_to_start();
        } else if(c == 'B' /*down*/){
            if( E.row_no < record_keeper.size()-1 ) E.row_no++;
            the_path = E.current_path.c_str();
            display_arr_on_terminal(E.row_no, record_keeper);
            reposition_cursor_to_start();
        } else if(c == 'C' /*right*/){}
        else if(c == 'D' /*left*/){}
        else if( c==10 ){}
    }
  return 0;
}