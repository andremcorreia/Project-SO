#include "logging.h"

#include "logging.h"
#include "operations.h"

#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <pthread.h>

void publisher(char* clientPipe,char* boxName){
    int f = tfs_open(boxName,TFS_O_APPEND);
    if (f == -1){
        printf("error to do in L126 - mbroker.c\n");
        return;
    }
    char msg[1024];
    int receivingPipe = open(clientPipe, O_RDONLY);
    while (true)
    {    
        ssize_t readBytes = read(receivingPipe, msg, sizeof(msg));
        if (readBytes == -1)
        {
            printf("error to do 30 mbroker\n");
        }
        
        close(receivingPipe);
        tfs_write(f, msg, sizeof(msg));
        if (strlen(msg) == sizeof(msg))
            printf("no /o here\n");
        printf("wrote: %s\n",msg);
    }
    tfs_close(f);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    if (argc != 3){
        fprintf(stderr, "usage: mbroker <pipename> <max_sesions>\n");
    }

    tfs_init(NULL);

    //long threadCount = (long)argv[2];

// Remove pipe if it already exists
    if (unlink(argv[1]) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Create pipe
    if (mkfifo(argv[1], 0666) != 0) { // permissions changed to allow read and write for all users
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int receivingPipe = open(argv[1], O_RDWR);
    while (true) {

        signal(SIGPIPE, SIG_IGN);

        int sig;
        sigwait(&set, &sig);
        if(sig == SIGUSR1)

        char buffer[1024];
        ssize_t readBytes = read(receivingPipe, buffer, sizeof(buffer));
        if (readBytes == -1) {
            if (errno == EPIPE)
                printf("pipe closed\n");
            else
                printf("read failed with error: %s\n", strerror(errno));
            break;
        }
        //if (readBytes == 0) // if pipe is closed by other end
        //    break;
        //buffer[readBytes] = '\0'; // null terminate the buffer                                                   
        int code = 0;
        sscanf(buffer, "code = %d|%s",&code , buffer);
        char clientPipe[256];
        char boxName[32];
        switch (code)
        {
            case 1:
                sscanf(buffer, "%[^|]|%s",clientPipe , boxName);
                publisher(clientPipe,boxName);
                break;

            case 2:
                sscanf(buffer, "%s|%s",clientPipe , boxName);
                //subscriber()
                break;

            case 3:
                sscanf(buffer, "%[^|]|%s",clientPipe , boxName);
                int f = tfs_open(boxName, TFS_O_CREAT);
                if (f == -1){
                    printf("error to do 102 mbroke\n");
                }
                printf("ayo Boxed\n");
                fprintf(stdout, "OK\n");
                tfs_close(f);
                int managerPipe = open(clientPipe, O_WRONLY);
                char sendBuffer[1024];
                sprintf(sendBuffer, "code = 4|%d|%s", 0, "failed");                                 //this is not right
                ssize_t w = write(managerPipe, sendBuffer, 1024);
                if (w < 0) {                                                                      //maybe remove
                    fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                close(managerPipe);
                break;
            
            case 5:
                sscanf(buffer, "%[^|]|%s",clientPipe , boxName);
                tfs_unlink(boxName);
                printf("ayo unBoxed\n");
                fprintf(stdout, "OK\n");
                break;

            case 7:
                sscanf(buffer, "%s",clientPipe);
                //list_boxes()
                break;
            
            default:
                break;
        }
        
    }
/*    pthread_t tid[threadCount];
    
    for (int i = 0; i < threadCount; i++)
    {
        if (pthread_create(&tid[i], NULL, NULL, (void*) NULL) != 0) {
            printf("Failed while creating the threads\n");
            return -1;
        }
    }
    



    for (int i = 0; i < threadCount; i++)
    {
        pthread_join(tid[i], NULL);
    }
    */
    return -1;
    
}       

//signal(SIGPIPE,SIGIGN)
//if errno = EPIPE -> open wr