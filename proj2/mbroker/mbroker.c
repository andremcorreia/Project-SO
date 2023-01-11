#include "logging.h"

#include <pthread.h>

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    if (argc != 3){
        fprintf(stderr, "usage: mbroker <pipename> <max_sesions>\n");
    }

    printf("%d\n",(int)argv[2]);

    int threadCount = (int)argv[2];

    int registerFifo[1];

    if (pipe(registerFifo) != 0) {
        printf("[ERR]: pipe() failed: %s\n");
        exit(EXIT_FAILURE);
    }

    pthread_t tid[threadCount];
    
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
    return -1;
}       

//signal(SIGPIPE,SIGIGN)
//if errno = EPIPE -> open wr