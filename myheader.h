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

void render_blank_screen(); 
void die(const char *s);
void enable_canonical_mode();
void enable_non_canonical_mode();
void initialise_terminal();
int get_terminal_rows_and_cols(int *rows, int *cols);
void reposition_cursor_to_start();
int get_files(const char* pathname);
void gotoxy(int x, int y);