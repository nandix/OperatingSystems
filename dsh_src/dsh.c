/***************************************************************************//**
 * @mainpage Operating Systems -- Diagnostics Shell
 *
 * @section course_section Course Information
 *
 * @author Joe Lillo
 *
 * @date March 3, 2014
 *
 * @par Instructor:
 *         Dr. Karlsson
 *
 * @par Course:
 *         CSC 456 - Section 1 - 2:00 pm
 *
 * @par Location:
 *         McLaury - Room 313
 *
 *
 * @section compile_section Compiling and Usage
 *
 * @par Compiling Instructions:
 *      A makefile is included with the source code. Simply run make to
 * invoke gcc with the correct arguments.
 *
*******************************************************************************/

/************************************************************************//**
 *  @file dsh.c
 *
 *  @brief Main entry point for diagnostics shell
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include "prog1.h"
#include "prog2.h"
#include "prog3.h"
#include <stdlib.h>
#include "helperfunctions.h"
#include <unistd.h>

void handleCommand(int argc, char ** argv);

//i made a change

/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Entry point of program. Displays dsh> prompt in a
 * loop to get commands from the user. Makes a call
 * to handleCommand to handle the given commands
 * and arguments.
 *
 * @returns Status
 ******************************************************************************/
int main(void)
{
    getcwd(_START_CWD,800);

    // Register signals to be handled.
    startCatchSignals();

    // Used for user input.
    char * input = NULL;
    char ** args;
    int words;

    // Main program loop.
    do
    {
        // Delete previous input.
        if (NULL != input)
        {
            free(input);
            input = NULL;
        }

        // Shell prompt.
        printf("dsh> ");

        // Get user input.
        input = getInput();

        // Break input string into individual arguments.
        args = getArgs(input,&words);

        // Make a function call to handle user commands.
        handleCommand(words,args);

        // Free memory allocated for storing arguments.
        int i = 0;
        for (i = 0; i < words; i++)
        {
            free(args[i]);
        }
        free(args);

    }while(0 != strcmp(input,"exit"));

    onExit();

    return 0;
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Makes the appropriate function call to handle the command given in agrv[1]
 * with parameters argv[2]..argv[n]
 *
 * @param[in] argc - the number of arguments
 * @param[in] argv - a 2d array of characters containing the arguments.
 *
 * @returns 0
 ******************************************************************************/
void handleCommand(int argc, char ** argv)
{
    // Error checking
    if(argc < 1)
    {
        return;
    }
    if (NULL == argv[0])
    {
        return;
    }

    if ( 0 != isPipe(argc,argv) )
    {
        printf("\n");
        doPipe(argc,argv);
        printf("\n");
    }

    else if ( 0 != isRedirect(argc,argv) )
    {
        printf("\n");
        doRedirect(argc, argv);
        printf("\n");
    }

    else if ( 0 != isRemotePipe(argc,argv) )
    {
        printf("\n");
        doClient(argc,argv);
        printf("\n");
    }

    else if (0 == strcmp(argv[0],"cmdnm"))
    {
        cmdnm(argc,argv);
    }

    else if (0 == strcmp(argv[0],"signal"))
    {
        sendKill(argc,argv);
    }

    else if (0 == strcmp(argv[0],"systat"))
    {
        systat();
    }

    else if (0 == strcmp(argv[0], "pid"))
    {
        getPID(argc, argv);
    }

    else if (0 == strcmp(argv[0], "dserv"))
    {
        doServer(argc, argv);
    }

    else if (0 == strcmp(argv[0], "dclient"))
    {
        doClient(argc, argv);
    }

    else if (0 == strcmp(argv[0], "cd"))
    {
        if (0 != changeDirectory(argc, argv) && argc > 1)
        {
            printf("Cannot change to directory: %s\n", argv[1]);
        }
    }

    else if (0 == strcmp(argv[0], "mboxinit"))
    {
        startSharedMemory(argc,argv);
    }

    else if (0 == strcmp(argv[0], "mboxdel"))
    {
        stopSharedMemory();
    }

    else if (0 == strcmp(argv[0], "mboxread"))
    {
        readBox(argc,argv);
    }

    else if (0 == strcmp(argv[0], "mboxwrite"))
    {
        writeBox(argc,argv);
    }

    else if (0 == strcmp(argv[0], "mboxcopy"))
    {
        copyBox(argc,argv);
    }

    else if (0 == strcmp(argv[0],"exit"))
    {
        return;
    }

    else if (strlen(argv[0]) > 0)
    {
        printf("\n");
        if (0 != execCmd(argc,argv))
        {
            printf("Error executing command: %s\n",argv[0]);
        }
        printf("\n");
    }

}



