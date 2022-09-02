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
void change_dir(string path){
    char abs_path[2000];
    realpath(path.c_str(), abs_path);
    // string abs(abs_path);
    if(abs_path){
        chdir(abs_path);
        cout << abs_path << endl;
    } else cout << "wrong path" << endl;
}

int main(int argc, char **argv)
{
    string path = "./.././..";
    change_dir(path);
    return 0;
}