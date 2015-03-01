/************************************************************************//**
 *  @file prog2.c
 *
 *  @brief Function definitions from part 2 of dsh.
 ***************************************************************************/

#include "prog2.h"
#include "prog1.h"
#include "helperfunctions.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/resource.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Executes the command given in argv in a new process.
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - Process and arguments to execute.
 *
 * @return Status returned from executing the process.
 ******************************************************************************/
int execCmd(int argc, char ** argv)
{
    int pid;
    int status;
    struct rusage use;


    // Create a new process to execute the command.
    pid = fork();

    // Child process.
    if ( 0 == pid )
    {
        // Print some information
        printf("Child process created: pid = %d\n", getpid());
        printf("Output from command '%s':\n", argv[0]);
        printf("------------------------------------------\n");

        // Execute command.
        execvp(argv[0], argv);

        // If this executes the exec didn't work...
        exit(1);
    }

    // Wait for the child to exit.
    wait(&status);
    printf("------------------------------------------\n");
        
    // Print child process information.
    getrusage(RUSAGE_CHILDREN, &use);
    printf("Child process information:\n");
    printf("Child exited with status: %d\n", status);
    printf("User CPU Time: %ld.%06ld\n", use.ru_utime.tv_sec, use.ru_utime.tv_usec);
    printf("System CPU Time: %ld.%06ld\n", use.ru_stime.tv_sec, use.ru_stime.tv_usec);
    printf("Number of page faults: %ld\n", use.ru_majflt);
    printf("Number of swaps: %ld\n", use.ru_nswap);
    return status;
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Changes the working directory of the process.
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - argv[1] contains the directory to change to.
 *
 * @return Status from chdir command.
 ******************************************************************************/
int changeDirectory(int argc, char ** argv)
{
    // Error checking.
    if (argc < 2)
    {
        return -1;
    }

    // Change directory.
    return chdir(argv[1]);
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Pipes data from one process to another. This function expects arguments
 * int the form of [cmd1] [args1] | [cmd2] [args2]. Stdout from cmd1
 * is piped into stdin of cmd2.
 *
 * Note: This code was created by modifying the code found at this URL:
 * http://www.mcs.sdsmt.edu/ckarlsso/csc456/spring14/code/pipe1.c
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - Commands and pipe information.
 *
 * @return Status from the new process.
 ******************************************************************************/
int doPipe(int argc, char ** argv)
{
    int mPipe[2];
    int pid1, pid2;
    char * pipeCmd = NULL;
    char ** argv2 = NULL;
    int status;

    // Error checking
    if(argc < 3)
    {
        return 1;
    }

    // Finds the location of the | symbol in the arguments list.
    int pLoc = isPipe(argc,argv);

    // Can't find pipe (it shouldn't be at location 0...)
    if ( 0 == pLoc )
    {
        return 1;
    }

    // Splits the arguments list into two seperate lists
    // (on each side of the pipe).
    pipeCmd = argv[pLoc];
    argv[pLoc] = NULL;
    argv2 = &argv[pLoc + 1];

    // Create a new process
    pid1 = fork();

    // Child process
    if ( 0 == pid1)
    {

        // Create a pipe
        pipe(mPipe);

        // Create another process.
        pid2 = fork();

        // Grandchild process will run the given command and send the results
        // through the pipe.
        if ( 0 == pid2 )
        {
            printf("Process created to handle command '%s' (input to pipe): pid = %d\n", argv[0], getpid());

            // close stoud and replace it with input side of pipe.
            close(1);
            dup(mPipe[1]);
            close(mPipe[0]);
            close(mPipe[1]);

            // Execute the command.
            execvp(argv[0], argv);
            exit(1);
        }

        // The child process will recieve the output from the grandchild process...

        // Close stdin. Replace it with output side of pipe.
        close(0);
        dup(mPipe[0]);
        close(mPipe[0]);
        close(mPipe[1]);

        // Wait for the grandchild process to complete.
        wait(&status);

        // Print information...
        printf("Command '%s' finished with status: %d\n", argv[0],status);
        printf("Process created to handle command '%s' (recieves output from pipe): pid = %d\n", argv2[0], getpid());
        printf("Output from '%s' command:\n", argv2[0]);
        printf("------------------------------------------\n");

        // Execute command on right side of pipe symbol.
        execvp(argv2[0], argv2);
        exit(1);
    }

    // wait for child process to finish.
    wait(&status);

    printf("------------------------------------------\n");

    // Print process information.
    struct rusage use;
    getrusage(RUSAGE_CHILDREN, &use);

    printf("Child process information:\n");
    printf("Child exited with status: %d\n", status);
    printf("User CPU Time: %ld.%06ld\n", use.ru_utime.tv_sec, use.ru_utime.tv_usec);
    printf("System CPU Time: %ld.%06ld\n", use.ru_stime.tv_sec, use.ru_stime.tv_usec);
    printf("Number of page faults: %ld\n", use.ru_majflt);
    printf("Number of swaps: %ld\n", use.ru_nswap);

    // Restore this.
    argv[pLoc]=pipeCmd;

    return status;
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Given a list of arguments, this function determines if one of the arguments
 * is a pipe symbol (|) and returns the location of that character.
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - Commands and pipe information.
 *
 * @return Location of pipe character in argv.
 ******************************************************************************/
int isPipe (int argc, char ** argv)
{
    char ** i = argv;

    while ( 0 != strcmp(*i,"|") )
    {
        if (i == &argv[argc-1])
        {
            return 0;
        }

        i += 1;
    }

    return i-argv;
}

/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Redirects output from a process to a file or inputs a file into a process.
 * This function expects arguments in the following manner:
 * [cmd] [args] > [file] or [cmd] [args] < [file].
 * This function behaves in the same way as the standard linux terminal
 * redirect function.
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - Command, file, and redirect information.
 *
 * @return Status from the new process.
 ******************************************************************************/
int doRedirect(int argc, char **argv)
{
    int pid;
    char * redCmd;
    char ** argv2;
    int file = -1;
    int status;

    // Error checking
    if(argc < 3)
    {
        return 1;
    }

    // Determine if the > or < character is present and find the location
    // of it in argv.
    int pLoc = isRedirect(argc,argv);

    // Error checking.
    if ( 0 == pLoc )
    {
        return 1;
    }

    // Temporarily replace the > or < character with a NULL string.
    redCmd = argv[pLoc];
    argv[pLoc] = NULL;
    argv2 = &argv[pLoc + 1];


    // Create a new process.
    pid = fork();

    // Child process.
    if ( 0 == pid )
    {
        // If input to the process is coming from a file...
        if ( 0 == strcmp(redCmd,"<") )
        {
            // Open the file.
            file = open(argv2[0], O_RDONLY, S_IWUSR|S_IRUSR);

            // Close stdin and replace it with the opened file.
            if ( file != -1 )
            {
                printf("Process created to handle redirection. PID = %d\n", getpid());
                printf("Using file '%s' as input to command '%s'.\n", argv2[0],argv[0]);
                printf("Output:\n");
                printf("------------------------------------------\n");
                close(0);
                dup(file);
                close(file);
            }
            else
            {
                printf("Unable to open file '%s'\n", argv2[0]);
                exit(1);
            }
        }
        else // Output from the file is going to a file...
        {
            // Open the output file.
            file = open(argv2[0], O_CREAT|O_WRONLY, S_IWUSR|S_IRUSR);

            // Close stdout and replace it with the output file.
            if ( file != -1 )
            {
                printf("Process created to handle redirection. PID = %d\n", getpid());
                printf("Writing output from '%s' to file '%s'\n", argv[0], argv2[0]);
                printf("Output:\n");
                printf("------------------------------------------\n");
                close(1);
                dup(file);
                close(file);
            }
            else
            {
                printf("Unable to open file '%s'\n", argv2[0]);
                exit(1);
            }
        }

        // Execute the given command.
        execvp(argv[0], argv);
        exit(1);
    }

    // Wait for the child process to end and return the status.
    wait(&status);

    // Print child process information.
    struct rusage use;
    getrusage(RUSAGE_CHILDREN, &use);
    printf("------------------------------------------\n");
    printf("Child process information:\n");
    printf("Child exited with status: %d\n", status);
    printf("User CPU Time: %ld.%06ld\n", use.ru_utime.tv_sec, use.ru_utime.tv_usec);
    printf("System CPU Time: %ld.%06ld\n", use.ru_stime.tv_sec, use.ru_stime.tv_usec);
    printf("Number of page faults: %ld\n", use.ru_majflt);
    printf("Number of swaps: %ld\n", use.ru_nswap);

    return status;
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Determines if the < or > symbols exist as stings in the argv array.
 * Returns the index of the character.
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - Command, file, and redirect information.
 *
 * @return Index of the redirect character.
 ******************************************************************************/
int isRedirect (int argc, char ** argv)
{
    char ** i = argv;

    while ( 0 != strcmp(*i,">") && 0 != strcmp(*i,"<") )
    {
        if (i == &argv[argc-1])
        {
            return 0;
        }

        i += 1;
    }

    return i-argv;
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Determines if the < or > symbols exist as stings in the argv array.
 * Returns the index of the character.
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - Command, file, and redirect information.
 *
 * @return Index of the redirect character.
 ******************************************************************************/
int isRemotePipe(int argc, char ** argv)
{
    char ** i = argv;

    while ( 0 != strcmp(*i,"))") && 0 != strcmp(*i,"((")   )
    {
        if ( i == &argv[argc-1] )
        {
            return 0;
        }

        i += 1;
    }

    return i-argv;
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Starts a socket server to listen for clients. When a client connects,
 * this function spins off a new thread to service the client's requests.
 * After the client exits, this function will also exit. This will be changed
 * in the next version to continuously handle multiple client requrests.
 *
 * Note: This code was created by modifying the code found at this URL:
 * http://www.mcs.sdsmt.edu/ckarlsso/csc456/spring14/code/ALP-listings/chapter-5/socket-inet-server.c
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - Port information.
 *
 ******************************************************************************/
void doServer(int argc, char ** argv)
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    int ok;
    int port;
    int ret;
    pthread_t listenThread;
    void* status;

    if ( argc < 2 )
    {
        printf("Please specify port number\n");
        return;
    }
    port = strToInt(argv[1], &ok);
    if ( 0 != ok )
    {
        printf("Invalid port number\n");
        return;
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);

    printf("Waiting for client connection...\n");

    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    printf("Connection Established.\nCreating thread to handle connection...\n");

    ret = pthread_create(&listenThread, NULL, serverListen, (void *)&connfd);
    if ( 0 != ret )
    {
        printf("Could not create listener thread.\n");
        return;
    }

    pthread_join(listenThread, &status);

    printf("Joined threads.\n");

    close(connfd);
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * This function is meant to run in a thread. It simply watches the socket
 * connection to listen for client requests. On a new request, this function
 * forks a new process to handle the client request. The output from the
 * command is sent back across the socket.
 *
 * @param[in] arg - Socket connection descriptor.
 *
 ******************************************************************************/
void * serverListen(void *arg)
{
    int len;
    char recvBuff[2000];
    int pid = -1;
    char ** args;
    int words;
    //int i;
    int status;

    int * connptr = (int*)arg;
    int conn = *connptr;

    while(1)
    {
        len = read(conn, recvBuff, sizeof(recvBuff)-1);
        recvBuff[len] = '\0';

        if ( 0 == strcmp("exit",recvBuff) )
        {
            printf("Recieved exit command from client.\n");
            break;
        }
        printf("Recieved Command: %s\n",recvBuff);

        args = getArgs(recvBuff, &words);

        if (words > 0)
        {
            pid = fork();
            if ( 0 == pid )
            {
                printf("Created Process to handle client request. (PID = %d)\n", getpid());
                printf("Executing command: %s\n", args[0]);
                dup2( conn, STDOUT_FILENO );
                close(conn);
                execvp(args[0],args);
                printf("Command Failed.\n");
                exit(1);
            }
            wait(&status);
        }

//        for (i = 0; i < words; i++)
//        {
//            free(args[i]);
//        }
//        free(args);
    }

    pthread_exit(0);
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Starts a client socket connection on the given ip address and port. Also
 * starts a prompt for commands to send to the server. Spins off a thread to
 * listen for replies from the server. Typing 'exit' will exit the client
 * prompt and cause the server to shutdown as well.
 *
 * Note: This code was created by modifying the code found at this URL:
 * http://www.mcs.sdsmt.edu/ckarlsso/csc456/spring14/code/ALP-listings/chapter-5/socket-inet-client.c
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - IP address and port information.
 *
 * @returns Error Code (0 = success).
 ******************************************************************************/
int doClient(int argc, char ** argv)
{
    int sockfd = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr;
    int port = -1;
    int ok;
    char * in;
    int len;
    char ** args;
    int words;
    unsigned int i;
    pthread_t listenThread;
    void* status;
    int ret;

    if(argc < 3)
    {
        printf("Invalid Arguments.\n");
        return 1;
    }

    port = strToInt(argv[2], &ok);
    if ( 0 != ok )
    {
        printf("Invalid port number\n");
        return 1;
    }

    memset(recvBuff, '0', sizeof(recvBuff));

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    printf("Connection Established.\nCreating listening thread.\n");

    ret = pthread_create(&listenThread, NULL, clientListen, (void *)&sockfd);
    if ( 0 != ret )
    {
        printf("Could not create listener thread.\n");
        return 1;
    }

    while (1)
    {
        printf("[dsh]dclient> ");
        in = getInput();

        args = getArgs(in, &words);

        if (words > 0 && 0 == strcmp(args[0], "exit") )
        {
            pthread_cancel(listenThread); // maybe not great practice?
            write(sockfd,"exit",4);
            close(sockfd);
            break;
        }

        len = strlen(in);
        if (len >= 2000)
        {
            printf("Command caused buffer overflow -- (2000 character max)\n");
        }
        else
        {
            write(sockfd,in,len);
        }

        for (i = 0; i < words; i++)
        {
            free(args[i]);
        }
        free(args);

        free(in);
    }

    pthread_join(listenThread, &status);

    printf("Joined thread.\n");

    close(sockfd);

    return 0;
}

/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * This function is meant to run in a seperate thread. It simply listens to
 * a socket connection to recieve messages from the server. After recieving a
 * message, the text is printed to stdout.
 *
 * @param[in] arg - Socket connection descriptor.
 *
 ******************************************************************************/
void* clientListen(void *arg)
{
    int len;
    char recvBuff[2000];

    int * connptr= (int*)arg;
    int conn = *connptr;

    while(1)
    {
        len = read(conn, recvBuff, sizeof(recvBuff)-1);
        recvBuff[len] = '\0';

        printf("\n[Recieved Message from server]:\n");
        printf("------------------------------------------\n");
        printf("%s",recvBuff);
        printf("------------------------------------------\n");
    }


    pthread_exit(0);
}
