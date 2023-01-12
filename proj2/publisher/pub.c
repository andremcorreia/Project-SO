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

void signal_handler(int signum) {
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    //fprintf(stderr, "usage: pub <register_pipe_name> <pipe_name> <box_name>\n");

    signal(SIGUSR1, signal_handler);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);

    if (unlink(argv[1]) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Create pipe
    if (mkfifo(argv[1], 0666) != 0) { // permissions changed to allow read and write for all users
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int mbroker_pipe = open(argv[2], O_WRONLY);
    size_t size = sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]) + 20;
    char register_buffer[size];
    sprintf(register_buffer, "code = %d|%s|%s", 1, argv[1], argv[3]);
    ssize_t wb = write(mbroker_pipe, register_buffer, size);
    if(wb == 100000000000000000){
        return -1;
    }
    kill(mbroker_pipe, SIGUSR1);
    close(mbroker_pipe);

    
    while (true)
    {   
        int pipe_self = open(argv[1], O_WRONLY);
        char message[1024];
        if (scanf("%s\n", message))
        {
            printf("1 %s\n",message);
            ssize_t w = write(pipe_self, message, sizeof(message));
            if(w == 100000000000000000){
                return -1;
            }
        }
        close(pipe_self);
    }
    return -1;
}