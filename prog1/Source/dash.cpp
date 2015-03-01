/**
 * Program: dash
 *
 * Author: Dan Nix
 *
 * Description: 
 *     Dash is a shell that implements the following commands:
 *         - cmdnm
 *              Return the command string (name) that started 
 *              the process for a given process id
 * 
 *         - pid
 *              Return the process ids for a given command string
 *              (match all substrings)
 *             
 *         - systat
 *              Print out some process information
 *              print (to stdout) Linux version information, and 
 *              system uptime. 
 *              print memory usage information: memtotal and memfree. 
 *              print cpu information: vendor id through cache size. 
 *              using /proc/'*' files
 * 
 *         - exit
 *              Exits the shell
 */

// dirent.h is used for directories
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <vector>



using namespace std;

// Usage strings
const string CMDNM_USAGE = "cmdnm <pid>";
const string PID_USAGE = "pid <command>";
const string SYSTAT_USAGE = "systat";

const bool DEBUGGING = false;
/**
 * Function: parse_first_arg
 *
 * Description: returns the first argument provided by
 *                 the user ignoring extra spaces before and
 *                 after
 *
 * Args:
 *     string command: String provided by user
 *     string usage: command usage string to be output 
 *
 * Return:
 *     string arg: the first argument
 */

string parse_first_arg(string command, string usage)
{

    int first_space;    // index of first space in command
    int second_space;   // index of second space in command
    int start_index;

    string arg; // Argument to return

    // Get the position of the first space
    first_space = command.find_first_of("  \t\n");

    // If there's no space the user needs to enter an argument
    if( first_space == -1 ){
        cout << "Usage: " << usage << endl;
        return "";
    }

    // If there was an argument, figure out if we need to get a substring
    // to the end or just the next space
    start_index = command.find_first_not_of(" \t\n", first_space);
    second_space = command.find_first_of(" \t\n", start_index );

        
    if(DEBUGGING)
    {
        cout << "Start index " << start_index << endl;
        cout << "Final index " << second_space << endl;
    }

    // If the start and end index are the same then the argument was
    //  a bunch of spaces
    if(start_index == second_space)
    {
        cout << "Usage: " << usage << endl;
        return "";
    }

    // If the second space value was -1 we need to take the substring to 
    //  the end of string
    else if( second_space == -1 )
    {
        arg = command.substr(start_index);
    }
    else
    {

        arg = command.substr(start_index, second_space-start_index );
    }

    return arg;

}


/**
 * Function: cmdnm
 *
 * Description: returns the name of the process that started
 *                 the function with the pid provided by the user
 *
 * Args:
 *     string command: String provided by user
 */
void cmdnm(string command)
{
    // Get the argument
    ifstream fin;
    string prog_name;
    string file_name;

    // Get the first command line arg
    string arg = parse_first_arg(command, CMDNM_USAGE);

    if(DEBUGGING)
    {
        cout << "CMDNM ARG: |" << arg << "|" << endl;
    }

    // Check if the argument was an integer. If an empty string, there was no arg
    if( (!atoi(arg.c_str()) && arg.length() != 0) 
        || arg.find_first_not_of("1234567890") != string::npos ){
        cout << "Expected an integer pid. Got '" << arg << "'" << endl;
        return;
    }
    else if( arg.length() == 0 )
    {
        return;
    }

    // Open the process's file in /proc
    file_name = "/proc/" + arg + "/cmdline";
    fin.open( file_name.c_str() );

    // If we couldn't open the file, then the process pid isn't running
    if(!fin)
    {
        cout << "Process " << atoi(arg.c_str()) << " is not currently running" << endl;
        return;
    }

    // Get the first line of the status file and put in prog_name
    getline( fin, prog_name );

    // Comment out if you want to only print the process name, not its args
    //prog_name = parse_first_arg(prog_name, "Broke Program Name Parsing");
    // Replace nulls with spaces
    for( unsigned int j=0; j < prog_name.length(); j++ ){

        if( prog_name[j] == '\0' ){
            prog_name[j] = ' ';
        }
    }

    // TODO: Figure out how to get this thing to output with the weird characters
    //cout << "pid-> " << arg << ": " << prog_name << endl;
    printf("pid-> %6s: %-50s", arg.c_str(), prog_name.substr(0,50).c_str());

    if( prog_name.length() > 50 )
        printf("...(command >50 characters)");
    printf("\n");

    return;
}

/**
 * Function: pid
 *
 * Description: 
 *      returns the pid of all processes who's commands include the string provided
 *      by the user.
 *
 * Args:
 *     string command: String provided by user
 */
void pid(string command)
{
    vector<string> dir_names;

    string command_line;
    string file_name;
    string arg = parse_first_arg(command, PID_USAGE);

    // If no argument was provided, exit
    if(arg.length() == 0 )
    {
        return;
    }

    ifstream fin;

    if(DEBUGGING)
    {
        cout << "Arg: " << arg << endl;
    }


    // Get directory list for /proc
    DIR *dir_pointer;
    struct dirent *entry_pointer;
    dir_pointer = opendir("/proc");

    // If we got a valid directory pointer
    if (dir_pointer != NULL)
    {
        // Step through the directory and get pointer to each subdirectory
        while ((entry_pointer = readdir(dir_pointer)))
        {
            //Put the numeric filenames in the dir_names vector
            if( atoi(entry_pointer->d_name) )
            {
                dir_names.push_back(entry_pointer->d_name);
            }
        }
    }
    else
    {
        return;
    }

    // Go through direcotry list
    for(unsigned int i = 0; i < dir_names.size(); i++ ){
        // Open its cmdline file
        file_name = "/proc/"+dir_names[i]+"/cmdline";
        fin.open( file_name.c_str() );
        // Read file into string
        getline(fin, command_line);
        //cout << "CMD: " << command_line << endl;
        //cout << "command_line = " << command_line << endl;
        //command_line = parse_first_arg(command_line, "Broke Command Line Name Parsing");
        // if( the substring's start isn't -1 )
        
        if( DEBUGGING )
        {   
            cout << "Comparing: " << arg << " : " << command_line << endl;
            cout << "Loc: " << command_line.find(arg) << " : " << string::npos << endl;
        }

        if( command_line.find(arg) != string::npos ){
                // Replace nulls with spaces
                for( unsigned int j=0; j < command_line.length(); j++ ){

                    if( command_line[j] == '\0' ){
                        command_line[j] = ' ';
                    }
                }


                // Print the pid pid and full program name
                printf(">>> %6s: %-50s", dir_names[i].c_str(), command_line.substr(0,50).c_str());
                if( command_line.length() > 50 )
                    printf("...(command >50 characters)");
                printf("\n");
        }
        fin.close();
            
    }
    cout << endl;

    return;
}

/**
 * Function: systat
 *
 * Description: prints out system data and statistics
 *
 * Args:
 *     None
 */
void systat()
{
    ifstream fin;
    string output;

    // Extra space for easier readability
    cout << endl;

    // Get linux version info
    fin.open("/proc/version");

    if(!fin)
    {
        cout << "Could not open linux version information." << endl << endl;
    }
    else
    {
        getline( fin, output );
        cout << "Linux Version:\n" << output << endl << endl;
        fin.close();
    }

    // Get System Uptime
    fin.open("/proc/uptime");

    if(!fin)
    {
        cout << "Could not open uptime information." << endl << endl;
    }
    else
    {
        fin >> output;
        cout << "Uptime: " << output << endl << endl;
        fin.close();
    }
    cout << endl;

    // Get memory usage information: memtotal and memfree 
    fin.open("/proc/meminfo");

    if(!fin)
    {
        cout << "Could not open memory information." << endl;
    }
    else
    {
        cout << "Meminfo: " << endl;

        do{
            getline( fin, output );
            cout << "    " << output << endl;
        }while(output.substr(0,6) != "Cached");
        fin.close();
    }
    cout << endl;

    // Get cpu information: vendor id through cache size.
    fin.open("/proc/cpuinfo");

    if(!fin)
    {
        cout << "Could not open cpu information." << endl;
    }
    else
    {
        cout << "CPU Info: " << endl;

        do{
            getline( fin, output );
            cout << "    " << output << endl;
        }while(output.substr(0,10) != "cache size");

        fin.close();
    }
    cout << endl;


    return;
}

void print_usage_if_needed( bool invalid, string command ){

    // we want to print the dash program message if the command is not:
    //  - a blank line
    //  - cmdnm <arg>
    //  - pid <command> Taken care of 
    //  - systat   Taken care of in main
    //  - exit
    bool blank_line = false;
    bool exit_stmt = false;

    string dash_usage = "Invalid command: " + command + "\n" +
                        "Valid commands are: \n" +
                        "    " + CMDNM_USAGE     + "\n" +
                        "    " + PID_USAGE       + "\n" +
                        "    " + SYSTAT_USAGE    + "\n" +
                        "    exit\n"; 

    // If there's a blank line (only whitespace) we don't want to print usage messages
    if(command.size() == 0 || command.find_first_not_of(" \t\n")== string::npos)
    {
        blank_line = true;
        invalid = false;

    }
    
    if( command.substr(0, 5) == "exit " ||  
                command.substr(0, 5) == "exit\t" ||
                (command.substr(0,4) == "exit" && command.length() == 4 ) )
    {
        exit_stmt = true;
        invalid = false;
    }

    //  print the valid commands
    if( !blank_line && !exit_stmt && invalid )  
    {
            cout << dash_usage << endl;
    }

    return;
}

/**
 * Function: main
 *
 * Description: Main holds the loop to keep the shell running until
 *              "exit" is entered. 
 *
 * Args:
 *      argc: number of args
 *      argv: command line params (none for this program)
 */
int main(int argc, char const *argv[])
{
    // Initialize variable to use
    string command = "";
    bool invalid = true;
    int first_char;

    // Check for the command "exit" anywhere in line
    while(command.substr(0,5) != "exit " && command.substr(0,5) != "exit\t" 
        && !(command.substr(0,4) == "exit" && command.length() == 4 ) )
    {
        // Print the prompt
        cout << "dash> ";

        // Get the command from user
        getline(cin, command);

        if (cin.eof()==1) {
            cin.clear();
            cin.ignore();
            cout << endl;
            continue;
        }

        // Chop off leading whitespace
        if(command.find_first_not_of(" \t\n") != string::npos ){
            first_char = command.find_first_not_of(" \t\n");
            command = command.substr(first_char);
        }
        
        // See if the user tried to run cmdnm
        if(command.substr(0, 5) == "cmdnm")
        {
            if( command[5] == ' ' || command[5] == '\t' || command.size() == 5 )
            {
                cmdnm(command);
                invalid = false;
            }
        }
        // See if the user tried to run pid
        else if(command.substr(0,3) == "pid")
        {
            if(command[3] == ' ' || command[3] == '\t' || command.size() == 3 )
            {
                pid(command);
                invalid = false;
            }
        }
        // See if the user tried to run systat
        else if(command.substr(0,6) == "systat")
        {
            if(command[6] == ' ' || command[6] == '\t' || command.size() == 6 )
            {
                systat();
                invalid = false;
            }
        }

        // If we need to print the usage message, do it in a function
        print_usage_if_needed(invalid, command);

        // Assume the user's input is invalid by default
        invalid = true;

    }
    
    return 0;
}
