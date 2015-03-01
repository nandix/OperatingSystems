/************************************************************************//**
 *  @file prog2.h
 *
 *  @brief Function prototypes from part 2 of dsh.
 ***************************************************************************/

#ifndef PROG2_H
#define PROG2_H

// Execute a command in a new process.
int execCmd(int argc, char ** argv);

// Change working directory of process.
int changeDirectory(int argc, char ** argv);

// Pipe output from one command into the input of another.
int doPipe(int argc, char ** argv);

// Determine if arguments are requesting a pipe.
int isPipe (int argc, char ** argv);

// Do redirection between files and programs.
int doRedirect(int argc, char ** argv);

// Determine if arguments are calling for redirection.
int isRedirect (int argc, char ** argv);

// Determine if arguments are calling for remote pipes. (not used)
int isRemotePipe (int argc, char ** argv);

// Start a socket server.
void doServer(int argc, char **argv);

// Thread function for server.
void* serverListen(void* conn);

// Start a socket client.
int doClient(int argc, char **argv);

// Thread function for client.
void* clientListen(void* conn);

#endif
