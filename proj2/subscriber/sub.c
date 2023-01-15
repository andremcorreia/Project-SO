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
    //fprintf(stderr, "usage: sub <register_pipe_name> <pipe_name> <box_name>\n");
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

    uint8_t send_code = 2;
    int mbroker_pipe = open(argv[2], O_WRONLY);
    size_t size = sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]);
    void* register_buffer;

    register_buffer = malloc(sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
    if(register_buffer == NULL) {
        printf("Error: malloc failed\n");
        free(register_buffer);
        cleanPipe(argv[1]);
        exit(EXIT_FAILURE);
    }
    memset(register_buffer,0, sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
    memcpy(register_buffer, &send_code, sizeof(uint8_t));
    memcpy(register_buffer + sizeof(uint8_t), argv[1], sizeof(char[256]));
    memcpy(register_buffer + sizeof(char[256]) + sizeof(uint8_t), argv[3], sizeof(char[32]));
    ssize_t w = write(mbroker_pipe, register_buffer, size);
    if(w == -1){
        cleanPipe(argv[1]);
        exit(EXIT_FAILURE);
    }
    close(mbroker_pipe);

    int count = 0;
    int self_pipe = open(argv[1], O_RDONLY);
    while (active)
    {
        uint8_t code;
        char message[1024];
        ssize_t readBytes = read(self_pipe, &code, sizeof(uint8_t));
        if (!active)    //to stop it from repeating last line after SIGINT
            break;
        
        if (readBytes !=0){
            readBytes += read(self_pipe, message, sizeof(char[1024]));
            char *temp = message;
            int len = (int)strlen(message);
            while (len != 0) {
                count++;
                fprintf(stdout, "%.*s\n", len, temp);
                temp += len + 1;
                len = (int)strlen(temp);
            }
        }
        else{
            active = false;
        }
    }
    close(self_pipe);
    fprintf(stdout, "%d\n", count);
    cleanPipe(argv[1]);
    return 0;
}