/************************************************************************//**
 *  @file prog3.h
 *
 *  @brief Function prototypes from part 3 of dsh.
 ***************************************************************************/

#ifndef PROG3_H
#define PROG3_H

//#define USE_SHMEM_SOCKETS       // Use sockets to communicate shared
                                  // memory address.

// Use a blocking function in the critical section of the data writing
// function for testing purposes.
//#define BLOCK_WRITE getchar();      // Do block writes
#define BLOCK_WRITE               // Do not block writes


// Use a blocking function in the critical section of the readering
// function for testing.
//#define BLOCK_READ getchar();       // Do block reads
#define BLOCK_READ                // Do not block reads


// Extra debugging statements. (Uncomment one or the other)
//#define DEBUG_PROG3(str, num) printf("PROG3 DEBUG: %s -- %d\n",str,num);
#define DEBUG_PROG3(str, num)

// For unused parameters... gets rid of compiler warnings.
#define UNUSED(x) (void)(x)

// Used to store the working directory of the process on startup.
char _START_CWD[1000];

// -------- Shell intrinsic functions --------
// Create shared mailboxes.
int startSharedMemory(int argc, char ** argv);
// Delete shared mailboxes.
int stopSharedMemory();
// Read from a mailbox. Prints data to stdout.
int readBox(int argc, char ** argv);
// Write a message to a mailbox.
int writeBox(int argc, char ** argv);
// Copy data from one mailbox to another.
int copyBox(int argc, char ** argv);
// -------------------------------------------

// Cleans up shared memory on exit.
void onExit();

// Shared Memory Functions
int createMailboxes (int num, int size);

// Read data from a mailbox.
int readMailbox (int shmid, int boxID);

// Write data to a mailbox.
int writeToMailbox (int shmid, int boxID, char * message);

// Copy data from one mailbox to another.
int copyMailbox(int shmid, int fromBox, int toBox);

// Socket server for distributing shared memory information.
// Runs in a seperate thread.
void* shmemServer (void* conn);

// Used to connect to the shared memory socket server.
int shmemClient(char * cmd, void ** ret, int *retLen);

// Returns the address of the shared memory. -1 if no
// shared memory exists.
int getshmemAddr();

// Return the PID that started the shared memory.
int getshmemParent();

#endif
