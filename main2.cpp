#include "myheader.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <string.h>
#include <iostream>

#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

using namespace std;

// string convert_secs_to_days(){

// }

int main(){
    struct stat file_data;
    stat("/home/yash/Desktop/assignments", &file_data);
    string modified_time = ctime(&file_data.st_mtime);

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
    
    cout << username << endl;
    cout << groupname << endl;
    cout << permissions << endl;
    cout << modified_time << endl;
    cout << size_of_file << endl;
}

// cout<<((S_ISDIR(file_data.st_mode))  ? "d" : "-");
//     cout<<((file_data.st_mode & S_IRUSR) ? "r" : "-");           // S_IRUSR = ..00100000000 and its logical And with 
//     cout<<((str1.st_mode & S_IWUSR) ? "w" : "-");           // st_mode = 100000111111101 gives us the answer 
//     cout<<((str1.st_mode & S_IXUSR) ? "x" : "-");           // for the required place.
//     cout<<((str1.st_mode & S_IRGRP) ? "r" : "-");
//     cout<<((str1.st_mode & S_IWGRP) ? "w" : "-");
//     cout<<((str1.st_mode & S_IXGRP) ? "x" : "-");
//     cout<<((str1.st_mode & S_IROTH) ? "r" : "-");
//     cout<<((str1.st_mode & S_IWOTH) ? "w" : "-");
//     cout<<((str1.st_mode & S_IXOTH) ? "x" : "-");
//     cout<<("\t");