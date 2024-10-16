#include <iostream>
#include <getopt.h>
#include <fstream>
using namespace std;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                  GLOBAL VARIABLES
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

string BANK_AUTH_FILE_PATH = "./auth.txt";
string ATM_AUTH_CONTENT = "";
string IP_ADDRESS = "127.0.0.1";
int PORT = 8080;
string USER_CARD = "";
string ACCOUNT = "";
char MODE = '-';
double BALANCE = 0;
double AMOUNT = 0;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                  UTILITY FUNCTIONS
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
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
            IP_ADDRESS = optarg;
            break;
        case 'p':
            PORT = stoi(optarg);
            break;
        case 'c':
            USER_CARD = optarg;
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
            BALANCE = stod(optarg);
            break;
        case 'd':
            if (MODE == '-')
                MODE = 'd';
            else
            {
                error = "Specifying multiple modes is not allowed! \n";
            }
            AMOUNT = stod(optarg);
            break;
        case 'w':
            if (MODE == '-')
                MODE = 'w';
            else
            {
                error = "Specifying multiple modes is not allowed! \n";
            }
            AMOUNT = stod(optarg);
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

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                  MAIN FUNCTION
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{

    string error;

    parseArguments(argc, argv, error);

    if (error != "")
    {
        error += "Run ./atm -h to get help. \n";
        cerr << error;
        exit(255);
    }
    cout << "Account: " << ACCOUNT << endl;
    cout << "Card File: " << USER_CARD << endl;
    cout << "Mode: " << MODE << endl;
    cout << "Amount: " << AMOUNT << endl;
    cout << "Balance: " << BALANCE << endl;
    cout << "Auth File: " << ATM_AUTH_CONTENT << endl;
    cout << "IP Address: " << IP_ADDRESS << endl;
    cout << "Port: " << PORT << endl;
    return 0;
}