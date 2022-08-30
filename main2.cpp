#include "myheader.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <string.h>
#include <iostream>
// #include <algorithm>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

using namespace std;

bool is_directory(string path){
    struct stat file_data;
    const char* temp_path = path.c_str();
    stat(temp_path, &file_data);
    string is_dir="";
    is_dir += ((S_ISDIR(file_data.st_mode))  ? "d" : "-");
    if( is_dir=="-" ) return false;
    return true;
}

int main(){
    cout << is_directory("/home/yash/Desktop/assignments/pdfs/AOS_Assignment_1.pdf") << endl;
}