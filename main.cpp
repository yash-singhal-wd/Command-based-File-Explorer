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

using namespace std;

// function declarations
char read_input_by_byte();
void render_blank_screen(); 
void die(const char *s);
void enable_canonical_mode();
void enable_non_canonical_mode();
void initialise_terminal();
int get_terminal_rows_and_cols(int *rows, int *cols);
void reposition_cursor_to_start();
char read_input_by_byte();
void print_message(string s, int row);
int display_initial_files();

/**** Initial terminal attributes and related functions ****/
struct terminal_configs {
  int number_of_rows_terminal;
  int number_of_cols_terminal;
  struct termios orig_termios;
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

int display_initial_files(){
    DIR* dir = opendir(".");
    if( dir==NULL ){
        return 1;
    }

    struct dirent* entity;
    entity = readdir(dir);

    while(entity!=NULL){
        printf("%s\n", entity->d_name);
        entity = readdir(dir);
    }

    closedir(dir);
    return 0;
}

/**** input screen related ****/
char read_input_by_byte() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

int main() {
    enable_non_canonical_mode();
    initialise_terminal();
    render_blank_screen();
    reposition_cursor_to_start();
    display_initial_files();
    char c;
    while(1){
        c = read_input_by_byte();
        if( c=='q' ){
            render_blank_screen();
            reposition_cursor_to_start();
            break;
        } 
        if (iscntrl(c)) {
            // cout << "here" << endl;
            printf("%d\n", c);
        }
        else
            printf("%d ('%c')\n", c, c);
    }
  return 0;
}