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

bool copydir(string src, string dest){

	DIR * direct;
	struct dirent * d;
	struct stat info;
	direct = opendir(src.c_str());
	if (direct == NULL) return false;
	while ((d = readdir(direct)))
	{
		string sname = src + "/" + string(d->d_name);
		string dname = dest + "/" + string(d->d_name);
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
	closedir(direct);
    return true;
}



int main(int argc, char **argv)
{
    copydir("/home/yash/Desktop/new_dir", "/home/yash/Desktop/latest_dir");
}