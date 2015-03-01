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

// Includes from Karlsson's code
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <stdio.h> 

//Needed for getrusage
#include <sys/time.h>
#include <sys/resource.h>



using namespace std;

// Usage strings
const string CMDNM_USAGE = "cmdnm <pid>";
const string PID_USAGE = "pid <command>";
const string SYSTAT_USAGE = "systat";

const bool DEBUGGING = false;