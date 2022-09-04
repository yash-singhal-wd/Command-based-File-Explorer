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

string get_parent_directory(string path){
    int p2 = path.length();
    int i;
    for(i=p2-1; path[i]!='/'; --i);
    string parent = path.substr(0, i);
    if(i==0) parent = "/"; 
    return parent; 
}

int fileExists(const char *path){
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

bool delete_dir(string source_dir_path){
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
		else if (S_ISDIR(info.st_mode)) delete_dir(new_source);
		else delete_file_command(new_source);
	}
	closedir(dir);
	remove(source_dir_path.c_str());
    return true;
}


int main(int argc, char **argv)
{
    string source = "~/Desktop/latest_dir";
    cout << delete_dir(source) << endl;
}