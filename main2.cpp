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

string get_tilda_dir(){
    uid_t uid;
    passwd* pw;
    uid = geteuid();
    pw = getpwuid(uid);
    string dir(pw->pw_name);
    dir = "/home/" + dir;
    return dir; 
}

string get_parent_directory(string path){
    int p2 = path.length();
    int i;
    for(i=p2-1; path[i]!='/'; --i);
    string parent = path.substr(0, i);
    if(i==0) parent = "/"; 
    return parent; 
}

bool rename_command(string old_path, string filename){
    // take first arg realpath
    if(old_path!="/"){
        if(old_path[0]=='~'){
            string to_append = get_tilda_dir();
            old_path = to_append + old_path.substr(1, old_path.length()-1);
        }
        char abs_path[2000];
        realpath(old_path.c_str(), abs_path);
        // if(!is_safe_to_list(abs_path)) return false;
        string temp(abs_path);
        old_path = temp;
    }

    //get parent and append 2nd arg 
    string parent = get_parent_directory(old_path);
    string new_path = parent + "/" + filename;
    //rename(old,new)
    int status = rename(old_path.c_str(), new_path.c_str());
    if( status==-1 ) return false;
    else return true; 
}

int main(int argc, char **argv)
{
    cout<<"Directory "<< rename_command("./../../hello_2", "hello_3")<< endl;
}