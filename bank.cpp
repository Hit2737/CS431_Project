#include<iostream>
#include<getopt.h>
#include<fstream>
using namespace std;


// initializing some global variables

// default variables
const long long DEFAULT_PORT = 8080;
const string DEFAULT_AUTH_FILE = "auth.txt";

// variables to be used
long long port = -1;
string auth_file_address  = "";


// function to get the available port
long long Get_Available_Port(){

    // This needs to be implemented properly
    return DEFAULT_PORT;
}

// function to get the available path
string Get_Available_Path(){

    FILE* file = fopen(DEFAULT_AUTH_FILE.c_str(), "r");
    int path_length = DEFAULT_AUTH_FILE.length();
    int copy = 0;
    string UPDATED_PATH;

    // if the default file already exists, we will try to make a copy file for that
    while(file==NULL){
        copy++;
        UPDATED_PATH = (DEFAULT_AUTH_FILE.substr(0, path_length-4)+"("+to_string(copy)+")");
        file = fopen(UPDATED_PATH.c_str(), "r");
    }

    return UPDATED_PATH;
}


int main(int argc, char *argv[]) {

    int option;
    string error;

    // parsing the command line arguments
    while ((option = getopt(argc, argv, "a:")) != -1) {
        switch (option) {
            case 'p':
                port = stoll(optarg);
            case 'P':
                if (port==-1) port = stoll(optarg);
            case 's':
                auth_file_address = optarg;
            case 'S':
                if (auth_file_address=="") auth_file_address = optarg;
            default:
                error = "Invalid argument";
        }
    }

    // setting the default values if not provided
    if (port==-1){
        port = Get_Available_Port();
    }
    else if (auth_file_address==""){
        auth_file_address = Get_Available_Path();
    }

    return 0;
}