#include <stdio.h>
#include <string.h>
#include <dirent.h>

int main(){
    DIR* dir = opendir(".");
    if( dir==NULL ){
        return 1;
    }

    struct dirent* entity;
    entity = readdir(dir);

    while(entity!=NULL){
        printf("%s\n", entity->d_name);
        entity = readdir(dir);
    }

    closedir(dir);
}