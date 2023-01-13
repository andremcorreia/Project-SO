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

static void sig_handler(int signum){
    if(signum == SIGINT){
        printf("catched\n");
        active = false;
    }
}

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

    if(signal(SIGINT, sig_handler) == SIG_ERR){
        printf("error sig\n");
        exit(EXIT_FAILURE);
    }

    uint8_t send_code = 2;
    int mbroker_pipe = open(argv[2], O_WRONLY);
    size_t size = sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]);
    void* register_buffer;

    register_buffer = malloc(sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
    if(register_buffer == NULL) {
        printf("Error: malloc failed\n");
        free(register_buffer);
        exit(EXIT_FAILURE);
    }
    memset(register_buffer,0, sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
    memcpy(register_buffer, &send_code, sizeof(uint8_t));
    memcpy(register_buffer + sizeof(uint8_t), argv[1], sizeof(char[256]));
    memcpy(register_buffer + sizeof(char[256]) + sizeof(uint8_t), argv[3], sizeof(char[32]));
    ssize_t w = write(mbroker_pipe, register_buffer, size);
    if(w == 1000000000000000){
        return -1;
    }
    close(mbroker_pipe);

    int count = 0;
    while (active)
    {
        int receivingPipe = open(argv[1], O_RDONLY);
        uint8_t code;
        char message[1024];
        ssize_t readBytes = read(receivingPipe, &code, sizeof(uint8_t));
        if (readBytes !=0){
            count++;
            readBytes += read(receivingPipe, message, sizeof(char[1024]));
            fprintf(stdout, "%s\n", message);
            close(receivingPipe);
        }
        else{
            active = false;
        }
    }
    fprintf(stdout, "%d\n", count);
    return 0;
}