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



#include<bits/stdc++.h>
#include<getopt.h>
#include<fstream>
#include<sqlite3.h>
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
                                        MONEY ARITHMETIC FUNCTIONS
*/

void format_correction(string &money){
    
    long long ind = -1;
    for(int i=0; i<money.size(); i++){
        if (money[i] == '.'){
            if (ind==-1){
                ind = i;
            }
            else{
                money = "";
                return;
            }
        }
        else if(!isdigit(money[i])){
            money = "";
            return;
        }
    }
    
    if (ind < money.size()-3){
        money = "";
        return;
    }

    if (ind==-1){
        money.push_back('.');
        ind = money.size()-1;
    }
    while(money.size() - 3ll < ind){
        money.push_back('0');
    }
}



string add(string old, string delta){

    string ans = "";

    long long old_ptr = old.size()-1, delta_ptr = delta.size()-1, carry = 0;

    while(old_ptr>=0 && delta_ptr>=0){
        if (old[old_ptr]=='.' && delta[delta_ptr]=='.'){
            ans.push_back('.');
        }
        else{
            int sum_of_chars = int(old[old_ptr] - '0') + int(delta[delta_ptr] - '0') + carry;
            if (sum_of_chars >= 10){
                sum_of_chars -= 10;
                carry = 1;
            }
            else{
                carry = 0;
            }
            ans.push_back(char(sum_of_chars));
        }

        old_ptr--;
        delta_ptr--;
    }

    while(old_ptr>=0){
        int sum_of_chars = int(old[old_ptr] - '0') + carry;
        if (sum_of_chars >= 10){
            sum_of_chars -= 10;
            carry = 1;
        }
        else{
            carry = 0;
        }
        ans.push_back(char(sum_of_chars));
    }

    while(delta_ptr>=0){
        int sum_of_chars = int(delta[delta_ptr]-'0') + carry;
        if (sum_of_chars >= 10){
            sum_of_chars -= 10;
            carry = 1;
        }
        else{
            carry = 0;
        }
        ans.push_back(char(sum_of_chars));
    }


    reverse(ans.begin(), ans.end());
}


string sub(string old, string delta){

    string ans = "";

    if (old.size()<delta.size()){
        return "";
    }
    
    int borrow = 0;
    
    long long old_ptr = old.size()-1, delta_ptr = delta.size()-1;
    while(old_ptr>=0 && delta_ptr>=0){
        int sub_chars = int(old[old_ptr] - '0') - borrow - int(delta[delta_ptr] - '0');
        if (sub_chars < 0){
            borrow = 1;
            sub_chars += 10;
        }
        else{
            borrow = 0;
        }

        ans.push_back(sub_chars);
        old_ptr--;
        delta_ptr--;
    }

    if (old.size()==delta.size()){
        if (borrow){
            return "";
        }
    }
    
    reverse(ans.begin(), ans.end());

    return ans;
}



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
                                        DATABASE FUNCTIONS

*/


int executeSQL(sqlite3* DB, const string &sql, char** messageError){
    return sqlite3_exec(DB, sql.c_str(), NULL, 0, messageError);
}


void create_table(sqlite3* DB){

    string create_table_sql =   "CREATE TABLE IF NOT EXITSTS accounts ("
                                "NAME       TEXT    PRIMARY KEY     NOT NULL, "
                                "MONEY      TEXT    NOT NULL,"
                                "PASSWORD   TEXT    NOT NULL );";

    char* messageError;

    if (executeSQL(DB, create_table_sql, &messageError) != SQLITE_OK){
        cerr << "Table creation error :- " << messageError << endl;
        sqlite3_free(messageError);
    }
}


void create_account(sqlite3* DB, string name, string money, string password){

    char* messageError;

    string insert_account_sql = "INSTERT INTO accounts (NAME, MONEY, PASSWORD) VALUES ( " + name + ", " + money + "," + password + ")";

    if (executeSQL(DB, insert_account_sql, &messageError) != SQLITE_OK){
        cerr << "Account creation error :- " << messageError << endl;
        sqlite3_free(messageError);
    }

}


string withdraw(sqlite3* DB, string name, string delta, string password){
    
    char* messageError;

    string begin_transaction = "BEGIN TRANSACTION;";

    if (executeSQL(DB, begin_transaction, &messageError) != SQLITE_OK){
        cerr << "Begin transaction error :- " << messageError << endl;
        sqlite3_free(messageError);
        return NULL;
    }

    string withdraw_sql = "";

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