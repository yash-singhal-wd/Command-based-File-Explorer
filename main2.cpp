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
int dirExists(const char *path)
{
    struct stat info;

    if(stat( path, &info ) != 0)
        return 0;
    else if(info.st_mode & S_IFDIR)
        return 1;
    else
        return 0;
}

int main(int argc, char **argv)
{
    const char *path = "/home/yash/Desktop/assignments";
    printf("%d\n", dirExists(path));
    return 0;
}