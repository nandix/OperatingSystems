/************************************************************************//**
 *  @file prog1.c
 *
 *  @brief Function definitions from part 1 of dsh.
 ***************************************************************************/

#include "prog1.h"
#include <errno.h>
#include "helperfunctions.h"


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Registers signals to be caught. The range of
 * signals is given by MIN_SIG -> MAX_SIG in prog1.h
 ******************************************************************************/
void startCatchSignals()
{
    unsigned int i;
    for (i = MIN_SIG; i < MAX_SIG; i++)
    {
        signal(i,handleSignal);
    }
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description: Registers signals to default signal handling function.
 ******************************************************************************/
void stopCatchSignals()
{
    unsigned int i;
    for (i = MIN_SIG; i < MAX_SIG; i++)
    {
        // Default signal handling.
        signal(i,SIG_DFL);
    }
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Callback function for handling signals.
 * Simply prints the signal number that was caught.
 *
 * @param[in] sig - Signal number to handle.
 ******************************************************************************/
void handleSignal(int sig)
{
    printf("[SIGNAL] dsh recieved signal: %d.\n", sig);

    // Exit with these signals.
    if (11 == sig || 2 == sig)
    {
        exit(1);
    }
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Displays name and command line information for a
 * given PID.
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - argv[1] -- PID to display information for.
 ******************************************************************************/
void cmdnm (int argc, char ** argv)
{
    if (argc < 2 || NULL == argv[1])
    {
        return;
    }

    // Get process name and command line string that started it.
    // char * name = getProcName(argv[1]);
    char * cmdline = getProcCmdline(argv[1]);

    // Error checking
    if (NULL == cmdline)
    {
        printf("Cannot find pid: %s\n",argv[1]);
    }
    else
    {
        // Print process information to stdio
        //printf(name);
        //free(name);
        printf("%s\n", cmdline);
        free(cmdline);
    }

}

/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Sends a signal to a given PID via the kill function.
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - argv[1] is the signal to send to the PID in argv[2].
 ******************************************************************************/
void sendKill(int argc, char ** argv)
{
    // Error checking
    if (argc < 3 || NULL == argv[1] || NULL == argv[2])
    {
        printf("Unable to send signal.\n");
        return;
    }

    // Get signal and process numbers from argv[1] and argv[2]
    int sigOk, pidOk = -1;
    int sig = strToInt(argv[1], &sigOk);
    int pid = strToInt(argv[2], &pidOk);

    if(0 != sigOk || 0 != pidOk)
    {
        printf("Unable to send signal %s to process %s.\n", argv[1], argv[2]);
        return;
    }

    // Send signal to process.
    int ret = kill(pid,sig);

    if (0 == ret)
    {
        printf("Signal %s sent to process %s.\n", argv[1], argv[2]);
    }
    else
    {
        printf("Unable to send signal %s to process %s\n.", argv[1], argv[2]);
    }
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Display system information.
 ******************************************************************************/
void systat()
{

    // ----------------- Version Information --------------

    FILE * fin = fopen("/proc/version","r");
    char in[100];

    if(!fin)
    {
        return;
    }

    fgets(in,100,fin);
    char * end = strchr(in,'(');
    *end = '\0';
    printf("%s\n",in);
    fclose(fin);

    // ----------------- System Uptime --------------
    fin = fopen("/proc/uptime","r");
    char time[100];

    if(!fin)
    {
        return;
    }

    fgets(time,100,fin);
    end = strchr(time,' ');
    *end = '\0';
    printf("Uptime: %s seconds\n",time);
    fclose(fin);

    // ----------------- Memory Information --------------
    fin = fopen("/proc/meminfo","r");

    if(!fin)
    {
        return;
    }

    fgets(in,100,fin);
    printf("%s",in);
    fgets(in,100,fin);
    printf("%s",in);

    fclose(fin);

    // ----------------- CPU Information --------------
    fin = fopen("/proc/cpuinfo","r");

    if(!fin)
    {
        return;
    }

    unsigned int i = 0;
    for (i=0; i < 9; i++)
    {
        fgets(in,100,fin);
        printf("%s",in);
    }
    printf("\n");

    fclose(fin);
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Searches through running processes in an attempt
 * to match the given search string to a substring
 * of the name of a process.
 * Prints all of the PIDs that return a positive
 * result.
 *
 * @param[in] argc - Number of arguments in argv
 * @param[in] argv - String to represent name of process.
 ******************************************************************************/
void getPID(int argc, char ** argv)
{
    // Error checking.
    if (argc < 2 || argv[1] == NULL)
    {
        return;
    }

    // Open /proc directory.
    DIR * proc = opendir("/proc");

    if (NULL == proc)
    {
        return;
    }

    // Read /proc directory.
    struct dirent * entry = readdir(proc);
    char * procName;

    int ok;

    // Keep reading /proc directory looking for directories with numbers
    // as names. Open [dir]/status to obtain process name. If the given
    // search string is a substring of that process name, print the PID
    // to std out.
    do
    {
        if(DT_DIR == entry->d_type)
        {
            strToInt(entry->d_name,&ok);
            if (0 == ok)
            {
                procName = getProcName(entry->d_name);
                if (NULL != procName)
                {
                    if (NULL != strstr(procName, argv[1]))
                    {
                        printf("%s\n",entry->d_name);
                    }
                    free(procName);
                }
            }
        }
    }while((entry=readdir(proc)));

    closedir(proc);
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Returns the name of a process given by PID parameter.
 *
 * @param[in] pid - String representation of PID.
 ******************************************************************************/
char * getProcName(char * pid)
{
    // Create filename string.
    char * fname = malloc (strlen(pid) + sizeof(char)*15);
    memcpy(fname,"/proc/",6);

    char * temp = fname + 6;
    memcpy(temp,pid,strlen(pid));

    temp = temp + strlen(pid);
    memcpy(temp,"/status\0",7);

    fname[13+strlen(pid)] = '\0';

    // Open status file.
    FILE * fin = fopen(fname,"r");

    if (NULL == fin)
    {
        printf("%d\n",errno);
        return NULL;
    }

    char * name = malloc(sizeof(char)*100);

    // Read and return first line of file.
    fgets(name,100,fin);

    fclose(fin);
    free(fname);

    return name;
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Returns the command line string that started the given process.
 *
 * @param[in] pid - String representation of process ID.
 * @return Command line string that started pid.
 ******************************************************************************/
char * getProcCmdline(char * pid)
{
    // Create file name string.
    char * fname = malloc (strlen(pid) + sizeof(char)*16);
    memcpy(fname,"/proc/",6);

    char * temp = fname + 6;
    memcpy(temp,pid,strlen(pid));

    temp = temp + strlen(pid);
    memcpy(temp,"/cmdline\0",8);

    fname[14+strlen(pid)] = '\0';

    // Open cmdline file
    FILE * fin;
    fin = fopen(fname,"r");

    if (NULL == fin)
    {
        return NULL;
    }

    // Allocate memory and set all 300 characters to \0.
    char * cmdline = malloc(sizeof(char)*300);
    memset(cmdline,'\0',300);

    // Read first line of file.
    fgets(cmdline,300,fin);

    char* i = cmdline;
    int wasNull = 0;
    int done = 0;

    // Iterate through string, swapping \0 characters with spaces until
    // the end of the string.
    do
    {
        if('\0' == *i)
        {
            // If two \0 characters occur in a row exit the loop.
            if(wasNull)
            {
                done = 1;
            }
            else
            {
                wasNull = 1;
                *i = ' ';
            }
        }
        else
        {
            wasNull = 0;
        }
        i += 1;
    }while(!done);

    fclose(fin);
    free(fname);

    return cmdline;
}









