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
#include "dash.h"
#include "dash1_funcs.cpp"
#include "dash2_funcs.cpp"


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
    string output = "";
    bool invalid = true;
    int first_char;

    // Check for the command "exit" anywhere in line
    while( !exit_command(command) )
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

        // If the command wasn't one that we implemented or "exit", try to fork a  
        //  child process and run it.
        else if( !exit_command(command) && command.length()!=0 )
        {
            output = run_sys_call(command);
            cout << output << endl;

            invalid = false;
        }

        // If we need to print the usage message, do it in a function
        print_usage_if_needed(invalid, command);

        // Assume the user's input is invalid by default
        invalid = true;

    }
    
    return 0;
}
