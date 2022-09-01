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


int window_size = 10;
int start_ind = 0;
int end_ind=window_size;
int main(){
    vector <int> vec;
    for (int i=0; i<100; ++i)
        vec.push_back(i+1);

    char c;
    while(1){
        cout << "hello" << endl;
        c = cin.get();
        if( c=='q' ){
            break;
        }
        else if( c=='w'){
            if(start_ind>0) start_ind--;
        } else if( c=='s'){
            if(end_ind<window_size) end_ind++;
        }

        for( int i=start_ind; i<end_ind; ++i){
            cout << vec[i] << endl;
        }
    }
}