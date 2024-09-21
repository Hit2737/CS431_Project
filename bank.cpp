/**
 * @file bank.cpp
 * @brief This file contains the implementation of a simple bank server application.
 * 
 * The application initializes default values for port and authentication file path,
 * parses command line arguments to override these defaults, and provides functions
 * to get available port and path for the authentication file.
 * 
 * Global Variables:
 * - DEFAULT_PORT: The default port number (8080).
 * - DEFAULT_AUTH_FILE: The default authentication file name ("auth.txt").
 * - port: The port number to be used, initialized to -1.
 * - auth_file_address: The authentication file path, initialized to an empty string.
 * 
 * Functions:
 * - Get_Available_Port: Returns the available port number. Currently returns the default port.
 * - Get_Available_Path: Returns an available path for the authentication file. If the default file exists, it creates a copy with a unique name.
 * 
 * Main Function:
 * - Parses command line arguments to set the port and authentication file path.
 * - Sets default values if not provided via command line arguments.
 * 
 * Command Line Arguments:
 * - -p <port>: Sets the port number.
 * - -P <port>: Sets the port number if not already set.
 * - -s <auth_file>: Sets the authentication file path.
 * - -S <auth_file>: Sets the authentication file path if not already set.
 * 
 * Error Handling:
 * - If an invalid argument is provided, an error message "Invalid argument" is set.
 */



#include<iostream>
#include<getopt.h>
#include<fstream>
using namespace std;



// initializing some global variables

// default variables
const long long DEFAULT_PORT = 3000;
const string DEFAULT_AUTH_FILE = "bank.auth";

// variables to be used
long long port = -1;
string auth_file_address  = "";



/*
-----------------------------------------------------------------------------------------------------
                                       FUNTION DECLARATIONS
-----------------------------------------------------------------------------------------------------
*/



/*
                                        UTILITY FUNCTIONS
*/


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
    while(file!=NULL){
        copy++;
        UPDATED_PATH = (DEFAULT_AUTH_FILE.substr(0, path_length-5)+"("+to_string(copy)+").auth");
        file = fopen(UPDATED_PATH.c_str(), "r");
    }

    return UPDATED_PATH;
}

// checks if the string is a number
inline bool is_number(const std::string& s)
{
    for(char curChar : s){
        if (!isdigit(curChar)) return false;
    }
    return true;
}


// function to parse the command line arguments
int parse_arguments(int argc, char *argv[]) {
    int option;

    struct option long_options[] = {
        {"port", required_argument, nullptr, 'p'},
        {"PORT", required_argument, nullptr, 'P'},
        {"auth_file", required_argument, nullptr, 's'},
        {"AUTH_FILE", required_argument, nullptr, 'S'},
        {"help", no_argument, 0, 'h'},
        {nullptr, 0, nullptr, 0}
    };


    while((option = getopt_long(argc, argv, "p:P:s:S:h", long_options, nullptr)) != -1){
        switch(option){
            case 'p':
                if (is_number(optarg)){
                    port = stoll(optarg);
                }
                else{
                    cout<<"Invalid argument !\n\nPlease run \n./bank -h \t or \t ./bank --help "<<endl;
                    return 1;
                }
                break;
            case 'P':
                if (is_number(optarg)){
                    port = stoll(optarg);
                }
                else{
                    cout<<"Invalid argument !\n\nPlease run \n./bank -h \t or \t ./bank --help "<<endl;
                    return 1;
                }
                break;
            case 's':
                auth_file_address = optarg;
                break;
            case 'S':
                auth_file_address = optarg;
                break;
            case 'h':
                cout<<"Usage: ./bank [OPTION]..."<<endl;
                cout<<"Start a bank server\n\n";
                cout<<"-p, --port\t\t\tPort number to listen on\n";
                cout<<"-P, --PORT\t\t\tPort number to listen on\n";
                cout<<"-s, --auth_file\t\t\tPath to authentication file\n";
                cout<<"-S, --AUTH_FILE\t\t\tPath to authentication file\n";
                cout<<"-h, --help\t\t\tDisplay this help and exit\n";
                return 1;
            default:
                cout<<"Invalid argument !\n\nPlease run \n./bank -h \t or \t ./bank --help "<<endl;
                return 1;
        }
    }
    return 0;
}



/*
                                       TRANSACTION FUNCTIONS
*/








int main(int argc, char *argv[]) {

    // parsing the command line arguments
    // If the arguments are invalid function will return 1
    if (parse_arguments(argc, argv)){
        return 1;       
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