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
bool is_directory(string path){
    struct stat file_data;
    const char* temp_path = path.c_str();
    stat(temp_path, &file_data);
    string is_dir="";
    is_dir += ((S_ISDIR(file_data.st_mode))  ? "d" : "-");
    if( is_dir=="-" ) return false;
    return true;
}

bool search(string path, string filename)
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
			bool op = search(next, filename);
			if (op == true) return true;
		}
	}
	closedir(dir);
	return false;
}

void rename_command(){
    // take first arg
    // take 2nd arg
    // find their realpaths 
    //rename(old,new)
}

bool create_directory(string path, string dirname){
    if(path=="/") path = path + dirname;
    else{
        // if(path[0]=='~') path = 
        // else path = 
        path = path + "/" + dirname;

    }
    char abs_path[2000];
    realpath(path.c_str(), abs_path);
    const int new_dir_status = mkdir(abs_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (new_dir_status==-1) return false;
    else return true; 
}

int main(int argc, char **argv)
{
    string path = "/home/yash/Desktop";
    string dirname = "newdir3";
    // cout << search(path, "aos") << endl;
    cout << create_directory(path, dirname) << endl;

    return 0;
}