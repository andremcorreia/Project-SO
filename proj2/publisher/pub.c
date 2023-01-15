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

bool active = true;

void cleanPipe(char* pipe){
    if (unlink(pipe) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", pipe, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void sig_handler(int signum){
    if(signum == SIGINT){
        printf("catched\n");
        active = false;
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    //fprintf(stderr, "usage: pub <register_pipe_name> <pipe_name> <box_name>\n");
    signal(SIGPIPE, SIG_IGN);
    cleanPipe(argv[1]);

    // Create pipe
    if (mkfifo(argv[1], 0666) != 0) { // permissions changed to allow read and write for all users
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(signal(SIGINT, sig_handler) == SIG_ERR){
        printf("error sig\n");
        cleanPipe(argv[1]);
        exit(EXIT_FAILURE);
    }


    size_t size = sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]);
    uint8_t send_code = 1;
    int mbroker_pipe = open(argv[2], O_WRONLY);
    if (mbroker_pipe == -1)
    {
        close(mbroker_pipe);
        printf("No mbroker\n");
        cleanPipe(argv[1]);
        exit(EXIT_FAILURE);
    }
    void* register_buffer;
    register_buffer = malloc(sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
    if(register_buffer == NULL) {
        printf("Error: malloc failed\n");
        free(register_buffer);
        close(mbroker_pipe);
        cleanPipe(argv[1]);
        exit(EXIT_FAILURE);
    }
    memset(register_buffer,0, sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
    memcpy(register_buffer, &send_code, sizeof(uint8_t));
    memcpy(register_buffer + sizeof(uint8_t), argv[1], sizeof(char[256]));
    memcpy(register_buffer + sizeof(char[256]) + sizeof(uint8_t), argv[3], sizeof(char[32]));
    ssize_t wb = write(mbroker_pipe, register_buffer, size);
    free(register_buffer);
    if(wb == -1){
        close(mbroker_pipe);
        printf("Failed to register in mbroker\n");
        cleanPipe(argv[1]);
        exit(EXIT_FAILURE);
    }
    close(mbroker_pipe);

    
    int pipe_self = open(argv[1], O_WRONLY);
    while (active)
    {   
        char message[1024];
        if (fgets(message, sizeof(message), stdin) != NULL)
        {
            message[strcspn(message, "\n")] = 0;
            ssize_t w = write(pipe_self, message, sizeof(message));
            if(w == -1){
                printf("Conection denied by mbroker\n");
                break;
            }
        }
        else {
            printf("Received EOF (Ctrl-D)\n");
            break;
        }
    }
    close(pipe_self);
    cleanPipe(argv[1]);
    return 0;
}