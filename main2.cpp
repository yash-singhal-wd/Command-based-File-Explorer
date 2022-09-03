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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

vector<string> global_all;

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

string get_last_child(string path){
    int p2 = path.length();
    int i;
    for(i=p2-1; path[i]!='/'; --i);
    string child = path.substr(i+1, path.length()-i);
    if(i==0) child="";
    return child;
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
        // if(!is_safe_to_list(source_file_path)) return false;
    }
    cout << dest_dir_path << endl;
    dest_dir_path = dest_dir_path + "/" + get_last_child(source_file_path);
	FILE *copy_from, *write_to;
	copy_from = fopen(source_file_path.c_str(), "r");
	write_to = fopen(dest_dir_path.c_str(), "w");
	if (copy_from == NULL) {
        cout << "Here" << endl;
		return false;
    }
    if( write_to == NULL){
         cout << "Here i am" << endl;
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


int main(int argc, char **argv)
{
    cout << copyfile_command("../../a.txt", "../../labs") << endl;
}