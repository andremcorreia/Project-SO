#include "logging.h"

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

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    if (argc != 3){
        fprintf(stderr, "usage: mbroker <pipename> <max_sesions>\n");
    }

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
    printf("checkpoint 1\n");
    signal(SIGPIPE, SIG_IGN);

    int receivingPipe = open(argv[1], O_RDONLY);
    printf("checkpoint 1\n");
    while (true) {
        char buffer[1024];
        ssize_t readBytes = read(receivingPipe, buffer, sizeof(buffer));
        if (readBytes == -1) {
            if (errno == EPIPE)
                printf("pipe closed\n");
            else
                printf("read failed with error: %s\n", strerror(errno));
            break;
        }
        if (readBytes == 0) // if pipe is closed by other end
            break;
        buffer[readBytes] = '\0'; // null terminate the buffer
                                                         
        int code = 0;
        sscanf(buffer, "code = %d|%s",&code , buffer);
        char clientPipe[256];
        char boxName[32];
        switch (code)
        {
            case 1:
                sscanf(buffer, "%s|%s",clientPipe , boxName);
                //publisher()
                break;

            case 2:
                sscanf(buffer, "%s|%s",clientPipe , boxName);
                //subscriber()
                break;

            case 3:
                sscanf(buffer, "%s|%s",clientPipe , boxName);
                printf("ayo Boxed\n");
                //create_box()
                break;
            
            case 5:
                sscanf(buffer, "%s|%s",clientPipe , boxName);
                printf("ayo unBoxed\n");
                //remove_box()
                break;

            case 7:
                sscanf(buffer, "%s",clientPipe);
                //list_boxes()
                break;
            
            default:
                break;
        }

        printf("%s\n",buffer);


        
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