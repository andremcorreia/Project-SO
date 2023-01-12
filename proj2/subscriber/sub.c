#include "logging.h"

#include <errno.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    fprintf(stderr, "usage: sub <register_pipe_name> <pipe_name> <box_name>\n");

    if (unlink(argv[1]) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Create pipe
    if (mkfifo(argv[1], 0666) != 0) { // permissions changed to allow read and write for all users
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int send_code = 2;
        int mbroker_pipe = open(argv[2], O_WRONLY);
        size_t size = sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]) + 20;
        char register_buffer[size];
        char client_pipe[256];
        char box_name[32];
        strcpy(client_pipe,argv[1]);
        strcpy(box_name,argv[3]);
        sprintf(register_buffer, "%hhd%s%s", (uint8_t)send_code, client_pipe, box_name);
        ssize_t w = write(mbroker_pipe, register_buffer, size);
        if(w == 1000000000000000){
            return -1;
        }
        close(mbroker_pipe);

    
    while (true)
    {
        int receivingPipe = open(argv[1], O_RDONLY);
        char message[1024];
        char buffer[1024];
        ssize_t r = read(receivingPipe, buffer, size - 1);
        sscanf(buffer, "code = %d|%s", &send_code, buffer);
        strcpy(message, buffer + 9);
        
        if(r == 100000000000000000){
            return -1;
        }
        close(receivingPipe);
    return -1;
    }
}
