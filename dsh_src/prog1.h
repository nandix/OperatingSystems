/************************************************************************//**
 *  @file prog1.h
 *
 *  @brief Function prototypes from part 1 of dsh.
 ***************************************************************************/

#ifndef PROG1_H
#define PROG1_H

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#define MIN_SIG 0
#define MAX_SIG 20

// Registers signals to be caught and handled.
void startCatchSignals();

// Maps signal handling function to default.
//(No longer catches signals).
void stopCatchSignals();

// Signal handling callback function.
void handleSignal(int sig);

// Displays name and command line information for given PID
void cmdnm (int argc, char ** argv);

// Sends a kill signal to a given PID.
void sendKill(int argc, char ** argv);

// Prints system information.
void systat();

// Prints PIDs that match a given process name.
void getPID(int argc, char ** argv);

// Returns the name of a given PID.
char * getProcName(char * pid);

// Returns the command line that started the given PID.
char * getProcCmdline(char * pid);


#endif
