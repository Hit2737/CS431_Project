#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <jsoncpp/json/json.h>
#include <random>
#include <getopt.h>
#include <fstream>
#include <regex>
using namespace std;
using namespace Json;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                  GLOBAL VARIABLES
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

string BANK_AUTH_FILE_PATH = "./auth.txt";
string ATM_AUTH_CONTENT = "";
string IP_ADDRESS = "127.0.0.1";
int PORT = 3000;
#define BUFFER_SIZE 1024
string USER_CARD = "";
string ACCOUNT = "";
char MODE = '-';
double BALANCE = 0;
double AMOUNT = 0;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                  COMMUNICATION FUNCTIONS
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

void sendMessageToServer(const string &message, const string &ip, int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(255);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0)
    {
        cerr << "Invalid IP address" << endl;
        exit(255);
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection to the bank server failed");
        exit(255);
    }

    send(sockfd, message.c_str(), message.size(), 0);
    cout << "Sent to server: " << message << endl;

    int n = read(sockfd, buffer, BUFFER_SIZE - 1);
    if (n > 0)
    {
        buffer[n] = '\0';
        cout << "Server response: " << buffer << endl;
    }

    close(sockfd);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                  UTILITY FUNCTIONS
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ................................  HELP FUNCTION  ................................
void printHelp()
{
    ifstream file = ifstream("help.txt");
    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            cout << line << endl;
        }
        file.close();
    }
    else
    {
        cerr << "help.txt not found!" << endl;
    }
}

// ................................  VALIDATION FUNCTIONS  ................................

bool isValidPort(char *port)
{
    if (strlen(port) > 5)
    {
        return false;
    }
    for (int i = 0; i < strlen(port); i++)
    {
        if (!isdigit(port[i]))
        {
            return false;
        }
    }
    string port_str = port;
    int port_num = stoi(port_str);
    return port_num >= 1024 && port_num <= 65535;
}

bool isValidAmount(char *amount)
{
    if (strlen(amount) > 13)
    {
        return false;
    }

    bool decimal = false;
    int decimal_places = 0;

    for (int i = 0; i < strlen(amount); i++)
    {
        if (!isdigit(amount[i]))
        {
            if (amount[i] == '.' && !decimal)
            {
                decimal = true;
            }
            else
            {
                return false;
            }
        }
        else if (decimal)
        {
            decimal_places++;
        }
    }

    double amount_num = stod(amount);
    return amount_num >= 0.00 && amount_num <= 4294967295.99;
}

bool isValidIP(const string &ip)
{
    int num;
    char ch;
    istringstream stream(ip);

    for (int i = 0; i < 4; i++)
    {
        if (!(stream >> num) || num < 0 || num > 255)
        {
            return false;
        }

        if (i < 3)
        {
            if (!(stream >> ch) || ch != '.')
            {
                return false;
            }
        }
    }

    return stream.eof();
}

bool isValidFileName(const string &fileName)
{
    if (fileName.length() < 6 || fileName.length() > 127)
    {
        return false;
    }
    string extension = fileName.substr(fileName.length() - 5);
    if (extension != ".card")
    {
        return false;
    }
    string name = fileName.substr(0, fileName.length() - 5);
    if (name == "." || name == "..")
    {
        return false;
    }

    regex validPattern("^[-_.0-9a-z]+$");

    return regex_match(name, validPattern);
}

bool isValidAccountName(const string &account)
{
    if (account.length() < 1 || account.length() > 122)
    {
        return false;
    }

    regex validPattern("^[-_.0-9a-z]+$");

    return regex_match(account, validPattern);
}

// ................................  PARSING FUNCTIONS  ................................

void check_req_args(string &error)
{
    if (ATM_AUTH_CONTENT == "")
    {
        error += "ATM authentication failed! Missing argument -s [...]\n";
    }
    if (USER_CARD == "")
    {
        error += "User authentication failed! Missing argument -c [...] \n";
    }
    if (MODE == '-')
    {
        error += "No mode specified! Missing argument -n [...] or -d [...] or -w [...] or -g \n";
    }
    if (ACCOUNT == "")
    {
        error += "No account specified! Missing argument -a [...] \n";
    }
    else if (!isValidAccountName(ACCOUNT))
    {
        error += "Invalid account name! Account name must be alphanumeric and can contain only the following characters: -_. \n";
    }
}

void parseArguments(int argc, char *argv[], string &error)
{
    int opt;

    while ((opt = getopt(argc, argv, "hs:i:p:a:c:n:d:w:g")) != -1)
    {
        switch (opt)
        {
        case 'h':
            printHelp();
            exit(0);

        case 's':
            ATM_AUTH_CONTENT = optarg;
            break;

        case 'i':
            if (isValidIP(optarg))
            {
                IP_ADDRESS = optarg;
            }
            else
            {
                error = "Invalid IP address! IP address must be a valid IPv4 address. \n";
            }
            break;

        case 'p':
            if (isValidPort(optarg))
            {
                PORT = stoi(optarg);
            }
            else
            {
                error = "Invalid port number! Port number must be an integer between 1024 and 65535. \n";
            }
            break;

        case 'c':
            if (isValidFileName(optarg))
            {
                USER_CARD = optarg;
            }
            else
            {
                error = "Invalid card file name! Card file name must be a valid file name with a .card extension. \n";
            }
            break;
        case 'a':
            ACCOUNT = optarg;
            break;
        case 'n':
            if (MODE == '-')
                MODE = 'n';
            else
            {
                error = "Specifying multiple modes is not allowed! \n";
            }
            if (isValidAmount(optarg))
            {
                BALANCE = stod(optarg);
            }
            else
            {
                error = "Invalid balance! Balance must be a positive number with only upto 2 decimal places from 0.00 to 4294969295.99 \n";
            }
            break;
        case 'd':
            if (MODE == '-')
                MODE = 'd';
            else
            {
                error = "Specifying multiple modes is not allowed! \n";
            }
            if (isValidAmount(optarg))
            {
                AMOUNT = stod(optarg);
            }
            else
            {
                error = "Invalid amount! Amount must be a positive number with only upto 2 decimal places from 0.00 to 4294969295.99 \n";
            }
            break;
        case 'w':
            if (MODE == '-')
                MODE = 'w';
            else
            {
                error = "Specifying multiple modes is not allowed! \n";
            }
            if (isValidAmount(optarg))
            {
                AMOUNT = stod(optarg);
            }
            else
            {
                error = "Invalid amount! Amount must be a positive number with only upto 2 decimal places from 0.00 to 4294969295.99 \n";
            }
            break;
        case 'g':
            if (MODE == '-')
                MODE = 'g';
            else
            {
                error = "Specifying multiple modes is not allowed! \n";
            }
            break;
        default:
            cerr << "Invalid argument! Run ./atm -h to get help." << endl;
            exit(255);
        }
    }

    check_req_args(error);
}

// ................................  GENERATE RANDOM PASSWORD  ................................

string generateRandomPassword(int length = 32)
{
    const string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~!@#$%^&*()_+-=:;,.<>/?";
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<> distribution(0, characters.size() - 1);

    string password;
    for (int i = 0; i < length; ++i)
    {
        password += characters[distribution(generator)];
    }
    return password;
}

// ................................  ATM FUNCTIONALITIES ................................

void createNewAccount(const string &account, double balance, const string &cardFile, const string &ip, int port)
{
    if (balance < 10.00)
    {
        cerr << "Initial balance must be at least 10.00" << endl;
        exit(255);
    }

    ifstream infile(cardFile);
    if (infile.good())
    {
        cerr << "Card file already exists" << endl;
        exit(255);
    }

    string password = generateRandomPassword();

    ofstream outfile(cardFile);
    if (!outfile)
    {
        cerr << "Failed to create card file" << endl;
        exit(255);
    }
    outfile << password;
    outfile.close();

    Value jsonMessage;
    jsonMessage["account"] = account;
    jsonMessage["initial_balance"] = balance;
    jsonMessage["password"] = password;
    StreamWriterBuilder writer;
    string message = writeString(writer, jsonMessage);

    sendMessageToServer(message, ip, port);
}

void depositMoney(const string &account, double amount, const string &cardFile, const string &ip, int port)
{
    if (amount <= 0.00)
    {
        cerr << "Deposit amount must be greater than 0.00" << endl;
        exit(255);
    }

    ifstream infile(cardFile);
    if (!infile)
    {
        cerr << "Card file not found" << endl;
        exit(255);
    }

    string password;
    infile >> password;
    infile.close();

    Value jsonMessage;
    jsonMessage["account"] = account;
    jsonMessage["deposit"] = amount;
    jsonMessage["password"] = password;
    StreamWriterBuilder writer;
    string message = writeString(writer, jsonMessage);

    sendMessageToServer(message, ip, port);
}

void withdrawMoney(const string &account, double amount, const string &cardFile, const string &ip, int port)
{
    if (amount <= 0.00)
    {
        cerr << "Withdraw amount must be greater than 0.00" << endl;
        exit(255);
    }
    ifstream infile(cardFile);
    if (!infile)
    {
        cerr << "Card file not found" << endl;
        exit(255);
    }
    string password;
    infile >> password;
    infile.close();

    Value jsonMessage;
    jsonMessage["account"] = account;
    jsonMessage["withdraw"] = amount;
    jsonMessage["password"] = password;
    StreamWriterBuilder writer;
    string message = writeString(writer, jsonMessage);

    sendMessageToServer(message, ip, port);
}

void getBalance(const string &account, const string &cardFile, const string &ip, int port)
{
    ifstream infile(cardFile);
    if (!infile)
    {
        cerr << "Card file not found" << endl;
        exit(255);
    }
    string password;
    infile >> password;
    infile.close();

    Value jsonMessage;
    jsonMessage["account"] = account;
    jsonMessage["get_balance"] = true;
    jsonMessage["password"] = password;
    StreamWriterBuilder writer;
    string message = writeString(writer, jsonMessage);

    sendMessageToServer(message, ip, port);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                  MAIN FUNCTION
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{

    // ------------------- Parse the command line arguments --------------------------
    string error;

    parseArguments(argc, argv, error);

    if (error != "")
    {
        error += "Run ./atm -h to get help. \n";
        cerr << error;
        exit(255);
    }

    // ------------------- Perform the ATM operation based on the mode --------------------------

    switch (MODE)
    {
    case 'n':
        createNewAccount(ACCOUNT, BALANCE, USER_CARD, IP_ADDRESS, PORT);
        break;
    case 'd':
        depositMoney(ACCOUNT, AMOUNT, USER_CARD, IP_ADDRESS, PORT);
        break;
    case 'w':
        withdrawMoney(ACCOUNT, AMOUNT, USER_CARD, IP_ADDRESS, PORT);
        break;
    case 'g':
        getBalance(ACCOUNT, USER_CARD, IP_ADDRESS, PORT);
        break;
    default:
        cout << "Invalid Mode Specified" << endl;
        exit(255);
    }

    return 0;
}