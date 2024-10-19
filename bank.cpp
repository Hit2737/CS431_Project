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
#include<csignal>
#include<unistd.h>
#include<arpa/inet.h>
#include<nlohmann/json.hpp>
#include<jsoncpp/json/json.h>
using namespace std;
using namespace nlohmann;
using namespace Json;



// initializing some global variables

// default variables
const long long DEFAULT_PORT = 3000;
const long long BUFFER_SIZE = 1024;
const string DEFAULT_AUTH_FILE = "bank.auth";

// variables to be used
long long port = -1;
string auth_file_address  = "";
int server_fd;
sqlite3* DB;



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

    if (ind==-1){
        money.push_back('.');
        ind = money.size()-1;
    }

    long long money_sz = money.size();
    while(money_sz - 3ll < ind){
        money.push_back('0');
        money_sz = money.size();
    }


    for(int i=0; i<money_sz-4; i++){
        if (money.front() == '0'){
            money.erase(money.begin());
        }
        else break;
    }
    
    money = money.substr(0, ind + 3);
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
            ans.push_back(char(sum_of_chars + '0'));
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
        ans.push_back(char(sum_of_chars + '0'));
        old_ptr--;
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
        ans.push_back(char(sum_of_chars + '0'));
        delta_ptr--;
    }

    if (carry){
        ans.push_back('1');
    }

    long long ans_ptr = ans.size()-1;

    while(ans_ptr>=4){
        if (ans[ans_ptr]=='0'){
            ans.pop_back();
        }
        else{
            break;
        }
        ans_ptr--;
    }

    reverse(ans.begin(), ans.end());

    return ans;
}


string sub(string old, string delta){

    string ans = "";

    if (old.size()<delta.size()){
        return "";
    }
    
    int borrow = 0;
    
    long long old_ptr = old.size()-1, delta_ptr = delta.size()-1;
    while(old_ptr>=0 && delta_ptr>=0){

        if (old[old_ptr]=='.' && delta[delta_ptr]=='.'){
            ans.push_back('.');
        }
        else{
            int sub_chars = int(old[old_ptr] - '0') - borrow - int(delta[delta_ptr] - '0');
            if (sub_chars < 0){
                borrow = 1;
                sub_chars += 10;
            }
            else{
                borrow = 0;
            }

            ans.push_back(sub_chars + '0');
        }

        old_ptr--;
        delta_ptr--;
    }

    if (old.size()==delta.size()){
        if (borrow){
            return "";
        }
    }

    while(old_ptr>=0){
        int sub_chars = int(old[old_ptr] - '0') - borrow;

        if (sub_chars < 0){
            borrow = 1;
            sub_chars += 10;
        }
        else{
            borrow = 0;
        }

        ans.push_back(sub_chars + '0');
        old_ptr--;
    }



    long long ans_ptr = ans.size()-1;

    while(ans_ptr>=4){
        if (ans[ans_ptr]=='0'){
            ans.pop_back();
        }
        else{
            break;
        }
        ans_ptr--;
    }

    reverse(ans.begin(), ans.end());

    return ans;
}



/*
                                        UTILITY FUNCTIONS
*/


bool check_input_for_sql_injection(const string &s){
    for(char c:s){
        if (c=='\''){
            return false;
        }
    }
    return true;
}


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


int executeSQL(const string &sql, char** messageError){
    return sqlite3_exec(DB, sql.c_str(), NULL, 0, messageError);
}


string check_password(string name, string password){

    char* messageError;


    string sql = "SELECT PASSWORD FROM accounts WHERE NAME= \'" + name + "\';";
    sqlite3_stmt*  stmt;

    if (sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return "SQL prepare error: " + string(sqlite3_errmsg(DB)) + "\n";
    }


    string current_password;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        current_password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    } else {
        sqlite3_finalize(stmt);
        return "AC_NOT_EXISTS";
    }
    sqlite3_finalize(stmt);

    if (password == current_password){
        return "";
    }
    else return "Wrong password";
}


void create_table(){

    string create_table_sql =   "CREATE TABLE IF NOT EXISTS accounts ("
                                "NAME       TEXT    PRIMARY KEY     NOT NULL, "
                                "MONEY      TEXT    NOT NULL,"
                                "PASSWORD   TEXT    NOT NULL );";

    char* messageError;

    if (executeSQL(create_table_sql, &messageError) != SQLITE_OK){
        cerr << "Table creation error :- " << messageError << endl;
        sqlite3_free(messageError);
    }
}



/*
                                       TRANSACTION FUNCTIONS
*/



string create_account(string name, string money, string password){

    string final_message = "";
    char* messageError;

    string insert_account_sql = "INSERT INTO accounts (NAME, MONEY, PASSWORD) VALUES ( \'" + name + "\', \'" + money + "\',\'" + password + "\');";

    // Execute the SQL command
    if (executeSQL(insert_account_sql, &messageError) != SQLITE_OK) {
        // Handle specific errors
        if (string(messageError).find("UNIQUE constraint failed") != string::npos) {
            final_message = "Account already exists for name: " + name + "\n";
        } else {
            final_message = "Account creation error: " + string(messageError) + "\n";
        }
        sqlite3_free(messageError);
    }

    return final_message;
}


string get_balance(string name){
    char* messageError;


    string sql = "SELECT MONEY FROM accounts WHERE NAME= \'" + name + "\';";
    sqlite3_stmt*  stmt;

    if (sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return "SQL prepare error: " + string(sqlite3_errmsg(DB)) + "\n";
    }


    string current_balance;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        current_balance = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    } else {
        sqlite3_finalize(stmt);
        return "AC_NOT_EXISTS";
    }
    sqlite3_finalize(stmt);

    return current_balance;
}


string withdraw(string name, string delta){
    
    char* messageError;

    string final_message = "";
    string begin_transaction = "BEGIN TRANSACTION;";

    if (executeSQL(begin_transaction, &messageError) != SQLITE_OK){
        final_message = "Begin transaction error :- " + string(messageError) +"\n";
        sqlite3_free(messageError);
        return final_message;
    }

    string current_balance = get_balance(name);
    string error = current_balance;
    format_correction(current_balance);
    if (current_balance == ""){
        final_message = error;
        string rollback_transaction = "ROLLBACK;";
        executeSQL(rollback_transaction, &messageError);
        return final_message;
    }

    string new_balance = sub(current_balance, delta);

    if (new_balance.empty()){
        string rollback_transaction = "ROLLBACK;";
        executeSQL(rollback_transaction, &messageError);
        return "INSUFFICIENT_BALANCE";
    }

    // Update the account with the new balance
    string update_sql = "UPDATE accounts SET MONEY=\'" + new_balance + "\' WHERE NAME=\'" + name + "\';";

    if (executeSQL(update_sql, &messageError) != SQLITE_OK) {
        cerr << "Update balance error: " << messageError << endl;
        sqlite3_free(messageError);
        // Rollback transaction
        string rollback_transaction = "ROLLBACK;";
        executeSQL(rollback_transaction, &messageError);
        return "";
    }


    // Commit the transaction
    string commit_transaction = "COMMIT;";
    if (executeSQL(commit_transaction, &messageError) != SQLITE_OK) {
        cerr << "Commit transaction error: " << messageError << endl;
        sqlite3_free(messageError);
        return "";
    }

    return final_message;
}


string deposit(string name, string delta){
    
    char* messageError;

    string begin_transaction = "BEGIN TRANSACTION;";
    string final_message = "";

    if (executeSQL(begin_transaction, &messageError) != SQLITE_OK){
        final_message =  "Begin transaction error :- " +  string(messageError) + "\n";
        sqlite3_free(messageError);
        return final_message;
    }

    string current_balance = get_balance(name);
    string error = current_balance;
    format_correction(current_balance);
    if (current_balance == ""){
        final_message = error;
        string rollback_transaction = "ROLLBACK;";
        executeSQL(rollback_transaction, &messageError);
        return final_message;
    }


    string new_balance = add(current_balance, delta);

    // Update the account with the new balance
    string update_sql = "UPDATE accounts SET MONEY=\'" + new_balance + "\' WHERE NAME=\'" + name + "\';";

    if (executeSQL(update_sql, &messageError) != SQLITE_OK) {
        final_message = "Update balance error: " + string(messageError) + "\n";
        sqlite3_free(messageError);
        // Rollback transaction
        string rollback_transaction = "ROLLBACK;";
        executeSQL(rollback_transaction, &messageError);
        return final_message;
    }


    // Commit the transaction
    string commit_transaction = "COMMIT;";
    if (executeSQL(commit_transaction, &messageError) != SQLITE_OK) {
        final_message=  "Commit transaction error: " + string(messageError) +"\n";
        sqlite3_free(messageError);
        return final_message;
    }

    return final_message;
}





/*
                                        COMMUNICATION FUNCTIONS
*/


void handle_client(int client_socket) {

    char buffer[BUFFER_SIZE];
    
    while (true) {

        memset(buffer, 0, BUFFER_SIZE);
        int n = read(client_socket, buffer, BUFFER_SIZE - 1);

        if (n <= 0) {
            break; // Connection closed by the client
        }

        // Parse the JSON message from the client
        string message(buffer);

        // Print the received message from the client (ATM)
        cout << "Received from ATM: " << message << endl;

        json request;
        try {
            request = json::parse(message);
        } catch (...) {
            // Invalid JSON, ignore the request
            continue;
        }

        // Process the request based on the operation
        json response;
        string name = request["account"];
        string password = request.contains("password") ? request["password"] : "";
        string mode = request["mode"];

        // validate account and password

        if (!check_input_for_sql_injection(name)){
            string response_message = "Invalid input for Name\n";
            send(client_socket, response_message.c_str(), response_message.length(), 0);
            cout<<"Invalid input for Name :- " + name + "\n";
            continue;
        }

        if (!check_input_for_sql_injection(password)){
            string response_message = "Invalid input for Password\n";
            send(client_socket, response_message.c_str(), response_message.length(), 0);
            cout<<"Invalid input for Password :- " + password + "\n";
            continue;
        }

        if (mode != "n"){
            string pass_check = check_password(name, password);
            if (pass_check != ""){
                string response_message = "Error Occured while checking the password :- " + pass_check + "\n";
                send(client_socket, response_message.c_str(), response_message.length(), 0);
                cout<<"Error Occured :- " + pass_check + " while checking the password for the account :- " + name + "\n";
                continue;
            }
        }


        if (mode=="n"){
            // new account
            string money = to_string(request["initial_balance"]);
            string response_message;

            format_correction(money);
            if (money==""){
                response_message = "Invalid input for money\n";
                send(client_socket, response_message.c_str(), response_message.length(), 0);
                cout<<"Invalid input for Balance :- " + money + "\n";
                continue;
            }

            string error = create_account(name, money, password);

            if (error == ""){
                response_message =  "Account has been created successfully!\n"
                                    "Your details are as follows:-\n"
                                    "Account Name :- " + name +
                                    "\n Account Balance :- " + money + "\n";
                send(client_socket, response_message.c_str(), response_message.length(), 0);
                cout<<"Account with name :- " + name + " and Balance :- " + money + " is ceated successfully.\n"; 
                continue;
            }

            response_message = "Error Occured :- " + error + "\n";
            cout<<"Error occured :- " + error + " for creating the account " + name + " and balance " + money + "\n";
            send(client_socket, response_message.c_str(), response_message.length(), 0); 

        }
        else if(mode == "d"){
            // deposit

            string delta = to_string(request["amount"]);
            string response_message;

            format_correction(delta);
            if (delta == ""){
                response_message = "Invalid input for deposit amount\n";
                send(client_socket, response_message.c_str(), response_message.length(), 0);
                cout<<"Invalid input for deposit amount :- " + delta + "\n";
                continue; 
            }

            string error = deposit(name, delta);

            if (error != ""){
                response_message = "Error Occured :- " + error + "\n";
                send(client_socket, response_message.c_str(), response_message.length(), 0);
                cout<<"Error Occured :- " << error << "\n";
                continue;
            }

            string changed_balance = get_balance(name);

            response_message = "Money deposited succesfully and your updated balance is " + changed_balance + "\n";
            cout<<"Money deposited from the account "+ name + " and the updated balance is " + changed_balance + "\n";
            send(client_socket, response_message.c_str(), response_message.length(), 0);
            
        }
        else if(mode == "w"){
            // withdraw

            string delta = to_string(request["amount"]);
            string response_message;


            format_correction(delta);
            if (delta == ""){
                response_message = "Invalid input for withdraw amount\n";
                send(client_socket, response_message.c_str(), response_message.length(), 0);
                cout<<"Invalid input for withdraw amount :- " + delta + "\n";
                continue; 
            }

            string error = withdraw(name, delta);

            if (error != ""){
                response_message = "Error Occured :- " + error + "\n";
                send(client_socket, response_message.c_str(), response_message.length(), 0);
                cout<<"Error Occured :- " << error << "\n";
                continue;
            }

            string changed_balance = get_balance(name);

            response_message = "Money withdrawed succesfully and your updated balance is " + changed_balance + "\n";
            cout<<"Money withdrawed from the account "+ name + " and the updated balance is " + changed_balance + "\n";
            send(client_socket, response_message.c_str(), response_message.length(), 0);

        }
        else if(mode == "g"){
            string response_message;
            string current_balance = get_balance(name);
            string error = current_balance;
            format_correction(current_balance);
            if (current_balance == ""){
                response_message = "Error Occured :- " + error + " for account :- " + name + "\n";
                cout<<"Error Occured :- " + error + " for account :- " + name + "\n"; 
                send(client_socket, response_message.c_str(), response_message.length(), 0);
                continue;
            }

            response_message = "Your Current Balance is :- " + current_balance + "\n";
            cout<<"Balance queried for the account :- " + name + " and its balance is :- "+ current_balance + "\n";
            send(client_socket, response_message.c_str(), response_message.length(), 0); 

        }
        else{
            close(client_socket);
        }

        
    }

    close(client_socket);
}


void handle_signal(int signal) {
    if (signal == SIGTERM) {
        cout << "Shutting down bank server..." << endl;
        close(server_fd);
        exit(0);
    }
}

// Function to start the bank server
void start_server(int port) {

    struct sockaddr_in address;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (server_fd == 0) {
        perror("Socket creation failed");
        exit(255);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(255);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(255);
    }

    cout << "Bank server listening on port " << port << "..." << endl;

    // Accept incoming connections and handle each one
    while (true) {
        int client_socket;
        socklen_t addr_len = sizeof(address);
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        if (fork() == 0) {
            close(server_fd); // Child process doesn't need the listener
            handle_client(client_socket);
            exit(0);
        } else {
            close(client_socket); // Parent process doesn't need this socket
        }
    }
}

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

    int exit = sqlite3_open("example.db", &DB);
    
    if (exit) {
        std::cerr << "Error open DB: " << sqlite3_errmsg(DB) << std::endl;
        return -1;
    } 

    create_table();

    // Set up signal handling for clean shutdown
    signal(SIGTERM, handle_signal);

    // Start the bank server
    start_server(port);


    return 0;
}