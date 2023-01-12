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

//void thread(char* clientPipe,char* boxName){
//    // inicializer
//    while(1) {
//        // consumir
//        // codigo trata publisher/sub/manager
//        // if/else para ver se e' publisher/sub...
//    }
//}

void publisher(char* clientPipe,char* boxName){
    int f = tfs_open(boxName,TFS_O_APPEND);
    if (f == -1){
        printf("error to do in L126 - mbroker.c\n");
        return;
    }
    char msg[1024];

    int receivingPipe = open(clientPipe, O_RDONLY);

    bool active = true;

    while (active)
    {    
        ssize_t readBytes = read(receivingPipe, msg, sizeof(msg));
        printf("%ld\n", readBytes);
        if (readBytes !=0)
        {
            tfs_write(f, msg, sizeof(msg));
            if (strlen(msg) == sizeof(msg))
                printf("no /o here\n");
            printf("wrote: %s\n",msg);
        }
        else{
            active = false;
        }
        
    }
    tfs_close(f);
    close(receivingPipe);

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
    signal(SIGPIPE, SIG_IGN);
    while (true) {
        uint8_t code = 0;
        ssize_t readBytes = read(receivingPipe, &code, sizeof(uint8_t));
        printf("code: %d\n", code);
        if (readBytes == 0)
                    break;
        char clientPipe[256];
        char boxName[320];
        switch (code)
        {
            case 1:
                readBytes += read(receivingPipe, clientPipe, sizeof(char[256]));
                readBytes += read(receivingPipe, boxName, sizeof(char[32]));
                printf("%s______%s\n",clientPipe,boxName);
                // produzir
                publisher(clientPipe,boxName);
                break;
            
            case 2:
                readBytes += read(receivingPipe, clientPipe, sizeof(char[256]));
                readBytes += read(receivingPipe, boxName, sizeof(char[32]));
                // produzir
                //subscriber(clientPipe,boxName);
                break;

            case 3:
                readBytes += read(receivingPipe, clientPipe, sizeof(char[256]));
                readBytes += read(receivingPipe, boxName, sizeof(char[32]));
                //printf("%s______%s\n",clientPipe,boxName);
                // produzir
                int f = tfs_open(boxName, TFS_O_CREAT);
                if (f == -1){
                    printf("error to do 102 mbroke\n");
                }
                printf("ayo Boxed\n");
                fprintf(stdout, "OK\n");
                tfs_close(f);
                int managerPipe = open(clientPipe, O_WRONLY);
                char sendBuffer[1024 + 2*sizeof(uint8_t)];
                char error[1024] = "failed";
                sprintf(sendBuffer, "%d%d%s",(uint8_t)4 , (uint32_t)0, error);                                 //this is not right
                printf("%s\n",sendBuffer);
                ssize_t w = write(managerPipe, sendBuffer, 1024);
                if (w < 0) {    
                    printf("aqui\n");                                                                  //maybe remove
                    fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                close(managerPipe);
                break;

            case 5:
                readBytes += read(receivingPipe, clientPipe, sizeof(char[256]));
                readBytes += read(receivingPipe, boxName, sizeof(char[32]));
                // produzir
                tfs_unlink(boxName);
                printf("ayo unBoxed\n");
                fprintf(stdout, "OK\n");
                break;

            case 7:
                readBytes += read(receivingPipe, clientPipe, sizeof(char[256]));
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