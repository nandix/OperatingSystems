/************************************************************************//**
 *  @file prog3.c
 *
 *  @brief Function definitions from part 3 of dsh.
 ***************************************************************************/

#include "prog3.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include "helperfunctions.h"
#include <semaphore.h>

#define K 1024
#define SHMKEY 1066
#define SOCKET_PORT 5000

/*!
 * \brief Information to describe a shared memory block.
 */
struct shmemInfo
{
    int boxSize;
    int numBoxes;
};

/*!
 * \brief Reader/Writer lock for a shared memory address.
 */
struct rwLock
{
    sem_t mutex;
    sem_t rw_mutex;
    int readCount;
};

/*!
 * \brief Wrapper function for creating shared memory.
 * \param argc - Number of arguments
 * \param argv - argv[1] = number of mailboxes. argv[2] = size of mailboxes.
 * \return Error code. 0 for success.
 */
int startSharedMemory(int argc, char ** argv)
{
    pthread_t thread;
    int ret;
    int ok;

    // Contains information about the shared memory block.
    struct shmemInfo *info = malloc(sizeof(struct shmemInfo));

    // Error checking
    if (argc < 3)
    {
        return -1;
    }

    // Check if server is already running...
    if( 0 < getshmemAddr())
    {
        printf("Shared memory already exists.\n");
        return -1;
    }

    // Get number of mailboxes to create.
    info->numBoxes = strToInt(argv[1],&ok);
    if (0 != ok)
    {
        return -1;
    }

    // Get size of mailboxes.
    info->boxSize = strToInt(argv[2], &ok);
    if (0 != ok)
    {
        return -1;
    }

    // Create a thread to handle the shared memory.
    // (Either a socket server or a file)
    ret = pthread_create(&thread, NULL, shmemServer, (void *)info);

    if (ret != 0)
    {
        printf("Error creating pthread.\n");
    }

    pthread_detach(thread);

    // Wait for the thread to create the shared memory.
    // Timeout after requesting the shared memory id too many times.
    unsigned int i = 0;
    int shmemid = getshmemAddr();
    while ( shmemid < 0 && i < 1000000 )
    {
        i++;
        shmemid = getshmemAddr();
    }

    if ( shmemid > 0 )
    {
        printf("New shared memory created:\n");
        printf("ID: %d\n", shmemid);
        printf("Number of mailboxes: %d\n",info->numBoxes);
        printf("Size of mailboxes: %d KB\n", info->boxSize);
    }
    else
    {
        printf("Timed out creating shared memory...\n");
    }
    return 0;
}

#ifdef USE_SHMEM_SOCKET
/*!
 * \brief Wrapper function to delete the shared memory block.
 * \param argc - Number of arguments.
 * \param argv - Note used.
 * \return Error code. 0 on success.
 */
int stopSharedMemory(int argc, char ** argv)
{
    char * ret;
    int retLen;

    //Send the message "exit" to the shared memory socket server.
    int ok = shmemClient("exit", &ret, &retLen);

    if (0 != ok)
    {
        return -1;
    }

    free(ret);

    printf("Shared memory marked for deletion.\n");

    return 0;
}
#else
/*!
 * \brief Wrapper function to delete the shared memory block.
 * \param argc - Number of arguments.
 * \param argv - Note used.
 * \return Error code. 0 on success.
 */
// Delete the shared memory information file.
int stopSharedMemory()
{
    char path[1000];
    int addr = getshmemAddr();

    if(0 < addr)
    {
        strcpy(path,_START_CWD);
        strcat(path,"/.dsh_shmem_info");
        shmctl(addr, IPC_RMID, 0);
        unlink(path);
    }
    else
    {
        return -1;
    }

    printf("Shared memory marked for deletion.\n");

    return 0;
}
#endif

/*!
 * \brief Wrapper function for reading a shared memory box.
 * \param argc - Number of arguments.
 * \param argv - argv[1] = box ID.
 * \return Error code. 0 on success.
 */
int readBox(int argc, char **argv)
{
    // Error checking.
    if (argc < 2)
    {
        return -1;
    }

    // Get mailbox number
    int ok;
    int box = strToInt(argv[1], &ok);
    if (0 != ok)
    {
        return -1;
    }

    // Get the shared memory address.
    int addr = getshmemAddr();
    if (addr > 0)
    {
        // Attempt to read from mailbox.
        if ( 0 != readMailbox(addr,box) )
        {
            printf("Invalid mailbox ID.\n");
        }
    }
    else
    {
        printf("No mailboxes exist.\n");
    }
    return 0;
}

/*!
 * \brief Wrapper function for writing to a shared memory mailbox.
 * \param argc - Number of arguments.
 * \param argv - argv[1] = Mailbox ID.
 * \return Error code. 0 on success.
 */
int writeBox(int argc, char ** argv)
{
    // Error checking
    if(argc < 2)
    {
        return -1;
    }

    // Get box ID.
    int ok;
    int box = strToInt(argv[1], &ok);
    if (0 != ok)
    {
        return -1;
    }

    // Get shared memory address.
    int addr = getshmemAddr();
    if (addr > 0)
    {
        // Get data to write to mailbox.
        printf("Enter message to write to box %d: ", box);
        char * msg = getInput();

        // Attempt to write to mailbox.
        if ( 0 !=  writeToMailbox(addr, box, msg))
        {
            printf("Invalid mailbox ID\n");
        }
        free(msg);
    }
    else
    {
        printf("No mailboxes exist.\n");
    }
    return 0;
}

/*!
 * \brief Wrapper function for copying data from one mailbox to another.
 * \param argc - Number of arguments
 * \param argv - argv[1] = from mailbox, argv[2] = to mailbox.
 * \return Error code. 0 on success.
 */
int copyBox(int argc, char **argv)
{
    // Error checking.
    if (argc < 3)
    {
        return -1;
    }

    // Get 'from' mailbox ID.
    int ok;
    int from = strToInt(argv[1], &ok);
    if (0 != ok)
    {
        return -1;
    }

    // Get 'to' mailbox ID.
    int to = strToInt(argv[2], &ok);
    if (0 != ok)
    {
        return -1;
    }

    // Get shared memory address.
    int addr = getshmemAddr();
    if (addr > 0)
    {
        // Attempt to copy data.
        if ( 0 != copyMailbox(addr,from,to) )
        {
            printf("Could not copy data from mailbox %d to mailbox %d.\n", from, to);
            return -1;
        }
    }
    else
    {
        printf("No mailboxes exist.\n");
        return -1;
    }

    printf("Data copied from mailbox %d to %d.\n",from, to);

    return 0;
}

/*!
 * \brief Delete shared memory on exit.
 */
void onExit()
{
    if(getshmemParent() == getpid())
    {
        printf("Deleting shared memory.\n");
        stopSharedMemory(0, NULL);
    }
}

#ifdef USE_SHMEM_SOCKETS
/*!
 * \brief Return the shared memory address (ID).
 * \return Shared memory ID.
 */
int getshmemAddr()
{
    char * ret;
    int retLen;

    // Contact the socket server for the shared memory ID>
    int ok = shmemClient("a", (void*)&ret, &retLen);

    if (0 != ok)
    {
        return -1;
    }

    // Get the address.
    int addr = *((int*)ret);

    free(ret);

    return addr;
}

/*!
 * \brief Returns the process ID that started the shared memory.
 * \return PID.
 */
int getshmemParent()
{
    char * ret;
    int retLen;

    int ok = shmemClient("parent", (void*)&ret, &retLen);

    if (0 != ok)
    {
        return -1;
    }

    int addr = *((int*)ret);

    free(ret);

    return addr;
}
#else

/*!
 * \brief Return the shared memory address (ID).
 * \return Shared memory ID.
 */
int getshmemAddr()
{
    char path[1000];
    FILE* f = NULL;
    int pid = -1;
    int addr = -1;

    // Create file path.
    strcpy(path,_START_CWD);
    strcat(path,"/.dsh_shmem_info");

    // Open shared memory information file.
    f = fopen(path,"r");
    if (f)
    {
        fread(&pid,sizeof(int),1,f);
        fread(&addr,sizeof(int),1,f);
        fclose(f);
    }
    else
    {
        DEBUG_PROG3("getshmemAddr: Shared memory does not exist\n",0);
    }

    return addr;
}

/*!
 * \brief Returns the process ID that started the shared memory.
 * \return PID.
 */
int getshmemParent()
{
    char path[1000];
    FILE* f = NULL;
    int pid = -1;
    int addr = -1;

    // Create file path.
    strcpy(path,_START_CWD);
    strcat(path,"/.dsh_shmem_info");

    // Open shared memory information file.
    f = fopen(path,"r");
    if (f)
    {
        fread(&pid,sizeof(int),1,f);
        fread(&addr,sizeof(int),1,f);
        fclose(f);
    }
    else
    {
        DEBUG_PROG3("getshmemParent: Shared memory does not exist\n",0);
    }

    return pid;
}
#endif

#ifdef USE_SHMEM_SOCKETS
/*!
 * \brief Socket server for handling shared memory information.
 * \param arg - Used to pass number and size of mailboxes to function.
 * \return
 */
void* shmemServer (void* arg)
{
    struct shmemInfo * info = arg;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    int len;
    char recvBuff[2000];

    // Get the PID that started the shared memory
    int parent = getpid();
    DEBUG_PROG3("Shared memory server pid",parent);

    printf("\n");

    // Create the shared memory.
    int shmid = createMailboxes(info->numBoxes, info->boxSize);
    if (shmid < 0)
    {
        return NULL;
    }

    // Setup socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SOCKET_PORT);
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 10);

    // Accept and process client requests.
    while (1)
    {
        // Accept client connection.
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        DEBUG_PROG3("Shared memory server accepted connection",connfd);

        // Read message from client.
        len = read(connfd, recvBuff, sizeof(recvBuff)-1);
        recvBuff[len] = '\0';

        // Handle exit command.
        if (0 == strcmp("exit",recvBuff))
        {
            DEBUG_PROG3("Shared memory server exiting",0);
            write(connfd, "OK\0", 3);
            break;
        }

        // Send PID that started the shared memory.
        else if (0 == strcmp("parent",recvBuff))
        {
            DEBUG_PROG3("Shared memory server writing parent ID",0);
            write(connfd, &parent, sizeof(int));
        }

        // Send the shared memory ID.
        else
        {
            DEBUG_PROG3("Shared memory server writing shared memory id",0);
            write(connfd, &shmid, sizeof(int));
        }

        close(connfd);
    }

    // Delete shared memory.
    shmctl(shmid, IPC_RMID, 0);

    shutdown(listenfd, 2);

    pthread_exit(0);
}
#else
/*!
 * \brief File server for handling shared memory information.
 * \param arg - Used to pass number and size of mailboxes to function.
 * \return
 */
// This implementation of the socket server uses a file
// to distribute shared memory information.
void* shmemServer (void* arg)
{
    char path[1000];
    struct shmemInfo * info = arg;
    FILE* f = NULL;

    // Construct the path to the shmem file.
    strcpy(path,_START_CWD);
    strcat(path,"/.dsh_shmem_info");

    // Determine if the file exists.
    f = fopen(path,"r");
    if (f)
    {
        fclose(f);
        printf("Error: Shared memory already exists.\n");
        return NULL;
    }

    // Open the file for writing.
    f = fopen(path, "w");
    if(NULL == f)
    {
        printf("Error creating shared memory info file.\n");
        return NULL;
    }

    // Create shared memory.
    int shmid = createMailboxes(info->numBoxes, info->boxSize);
    if (shmid < 0)
    {
        return NULL;
    }

    // Write PID and shared memory ID to file.
    int pid = getpid();
    fwrite(&pid,sizeof(int),1,f);
    fwrite(&shmid,sizeof(int),1,f);

    fclose(f);

    pthread_exit(0);
}
#endif


#ifdef USE_SHMEM_SOCKETS
/*!
 * \brief Used to connect with shared memory socket server.
 * \param cmd - Command to send to server.
 * \param ret - Return data from server.
 * \param retLen - Length of return data
 * \return Error code. 0 on success.
 */
int shmemClient(char * cmd, void ** ret, int *retLen)
{
    int sockfd = 0;
    char * recvBuff = malloc(2000);
    struct sockaddr_in serv_addr;
    int len;


    memset(recvBuff, '0', 2000);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SOCKET_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        //printf("\n Error : Connect Failed \n");
        return -1;
    }

    write(sockfd, cmd, strlen(cmd));
    len = read(sockfd, recvBuff, 1999);
    //recvBuff[len] = '\0';

    *ret = recvBuff;
    *retLen = len;

    close(sockfd);

    return 0;
}
#else
/*!
 * \brief Used to connect with shared memory socket server.
 * \param cmd - Command to send to server.
 * \param ret - Return data from server.
 * \param retLen - Length of return data
 * \return Error code. 0 on success.
 */
int shmemClient(char *cmd, void **ret, int *retLen)
{
    UNUSED(cmd);
    UNUSED(ret);
    UNUSED(retLen);
    return -1;
}
#endif

/*!
 * \brief Create shared memory
 * \param num - Number of mailboxes.
 * \param size - Size of mailboxes.
 * \return
 */
int createMailboxes (int num, int size)
{
    // Shared memory size.
    int infoLen = (sizeof(int)*2) + (sizeof(struct rwLock)*num) + (sizeof(char*)*num);
    int dataLen =  (num*size*K);

    // Obtain a shared memory ID.
    int shmid = shmget(SHMKEY, infoLen + dataLen , IPC_CREAT | IPC_EXCL | 0666);

    // Error checking.
    if ( shmid < 0)
    {
      printf("***ERROR: shmid is %d\n", shmid);
      perror("shmget failed");
      return -1;
    }

    DEBUG_PROG3("New shared memory ID",shmid);

    // Setup header data.
    char * addr =  shmat(shmid, 0, 0);

    int * info = (int*)addr;
    *info = num;
    info++;
    *info = size;

    // Reader/Write lock pointer.
    struct rwLock * lock = (struct rwLock*)(addr + sizeof(int)*2);

    // Iterate over reader/writer locks.
    int i = 0;
    for (i = 0; i < num; i++)
    {
        // Initialize reader/writer lock.
        if (0 != sem_init(&lock->rw_mutex,1,1))
        {
            printf("Error initializing rw mutex semaphore #%d\n",i);
            break;
        }
        // Initialize mutex lock.
        if (0 != sem_init(&lock->mutex,1,1))
        {
            printf("Error initializing mutex semaphore #%d\n",i);
            break;
        }
        lock->readCount = 0;
        lock ++;
    }

    // Release shared memory from this process.
    shmdt(addr);
    return shmid;
}

/*!
 * \brief Write data to a mailbox.
 * \param shmid - Shared memory ID.
 * \param boxID - Mailbox ID.
 * \param message - Data to write to mailbox.
 * \return error code - 0 on success.
 */
int writeToMailbox (int shmid, int boxID, char * message)
{
    char * addr =  shmat(shmid, 0, 0);
    int * temp = (int*)addr;
    int numBoxes = *temp;
    temp++;
    unsigned int size = *temp;

    // Error checking.
    if (boxID >= numBoxes || boxID < 0)
    {
        return -1;
    }

    // Address of reader/writer lock structor for mailbox.
    struct rwLock* lock = (struct rwLock*)(addr + sizeof(int)*2 + (sizeof(struct rwLock)*boxID));

    // Address of mailbox data.
    char * box = addr + sizeof(int)*2 + (sizeof(struct rwLock)*numBoxes) + (size*K)*boxID;

    // Display information about write.
    printf("Write addr: %p\n", box);
    printf("msg: %s\n", message);

    // If the message fits in the mailbox...
    if (strlen(message) < (size*K - 1))
    {
        // Obtain the read/write mutex for the box.
        sem_wait(&lock->rw_mutex);

        // Copy the message into the mailbox.
        BLOCK_WRITE
        memcpy(box, message, strlen(message));

        // Release the mutex.
        sem_post(&lock->rw_mutex);
    }
    else
    {
        // Obtain the read/write mutex for the box.
        sem_wait(&lock->rw_mutex);

        // Write the data (end at the max mailbox size).
        BLOCK_WRITE
        memcpy(box, message, size*K - 1);        
        box[size*K-1]='\0';

        // Release the mutex.
        sem_post(&lock->rw_mutex);

        printf("Message length of size %d is greater than mailbox size %d KB.\n Truncating message to %d KB.\n",(int)strlen(message),size,size);
    }

    shmdt(addr);

    return 0;
}

/*!
 * \brief Read data from a shared mailbox. Write to stdout
 * \param shmid - Shared memory ID.
 * \param boxID - Mailbox ID.
 * \return Error code - 0 on success.
 */
int readMailbox (int shmid, int boxID)
{
    char * addr =  shmat(shmid, 0, 0);
    int * temp = (int*)addr;
    int numBoxes = *temp;
    temp++;
    int size = *temp;

    // Error checking
    if (boxID >= numBoxes || boxID < 0)
    {
        return -1;
    }

    // Reader/Write lock structure for this mailbox.
    struct rwLock* lock = (struct rwLock*)(addr + sizeof(int)*2 + (sizeof(struct rwLock)*boxID));

    // Data address for this mailbox.
    char * box = addr + sizeof(int)*2 + (sizeof(struct rwLock)*numBoxes) + (size*K)*boxID;

    // Get reader mutex lock.
    sem_wait(&lock->mutex);

    // Increment the number of readers.
    lock->readCount++;

    // If this is the first reader lock the reader/writer mutex.
    if(lock->readCount == 1)
    {
        sem_wait(&lock->rw_mutex);
    }

    // Release the reader mutex.
    sem_post(&lock->mutex);

    // Perform read.
    BLOCK_READ
    printf("Read addr: %p\n", box);
    printf("Message: %s\n", box);

    // Obtain the reader mutex lock.
    sem_wait(&lock->mutex);
    // Decrement the reader count.
    lock->readCount--;

    // If there are no more readers, release the reader/writer mutex.
    if(lock->readCount == 0)
    {
        sem_post(&lock->rw_mutex);
    }

    // Release the reader mutex.
    sem_post(&lock->mutex);

    shmdt(addr);

    return 0;
}

/*!
 * \brief Copy data from one mailbox to another.
 * \param shmid - Shared memory ID.
 * \param fromBox - 'From' mailbox ID.
 * \param toBox - 'To' mailbox ID.
 * \return error code - 0 on success.
 */
int copyMailbox(int shmid, int fromBox, int toBox)
{
    char * addr =  shmat(shmid, 0, 0);
    int * temp = (int*)addr;
    int numBoxes = *temp;
    temp++;
    int size = *temp;

    // Error checking.
    if (fromBox >= numBoxes || fromBox < 0 ||
            toBox >= numBoxes || toBox < 0)
    {
        return -1;
    }
    if (fromBox == toBox)
    {
        return -1;
    }

    // R/W lock and data address for the "to" mailbox.
    struct rwLock* to_lock = (struct rwLock*)(addr + sizeof(int)*2 + (sizeof(struct rwLock)*toBox));
    char * to_boxAddr = addr + sizeof(int)*2 + (sizeof(struct rwLock)*numBoxes) + (size*K)*toBox;

    // R/W lock and data address for the "from" mailbox.
    struct rwLock* from_lock = (struct rwLock*)(addr + sizeof(int)*2 + (sizeof(struct rwLock)*fromBox));
    char * from_boxAddr = addr + sizeof(int)*2 + (sizeof(struct rwLock)*numBoxes) + (size*K)*fromBox;

    // wait on write lock for 'to' box. +++++++++++++
    sem_wait(&to_lock->rw_mutex);


    // ------ Wait on read lock for 'from' box ------
    sem_wait(&from_lock->mutex);
    from_lock->readCount++;
    if(from_lock->readCount == 1)
    {
        sem_wait(&from_lock->rw_mutex);
    }
    sem_post(&from_lock->mutex);

    // Copy data
    strcpy(to_boxAddr,from_boxAddr);

    sem_wait(&from_lock->mutex);
    from_lock->readCount--;
    if(from_lock->readCount == 0)
    {
        sem_post(&from_lock->rw_mutex);
    }
    sem_post(&from_lock->mutex);
    // ------- End read lock ---------


    sem_post(&to_lock->rw_mutex);
    // End write lock ++++++++++++++++++++++++++++++++++


    shmdt(addr);

    return 0;
}

