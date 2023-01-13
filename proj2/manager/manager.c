#include "logging.h"

#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

/*static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> <pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> <pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> <pipe_name> list\n");
}*/

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    //print_usage();

    signal(SIGPIPE, SIG_IGN);

    char* mode = argv[3];
    size_t size = sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]);

    if (unlink(argv[1]) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Create pipe
    if (mkfifo(argv[1], 0666) != 0) { // permissions changed to allow read and write for all users
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (!strcmp(mode, "create") || !strcmp(mode, "remove"))
    {
        uint8_t sendCode = 3;
        if (!strcmp(mode, "remove"))
            sendCode = 5;
        int mbroker_pipe = open(argv[2], O_WRONLY);

        void* register_buffer;
        
        register_buffer = malloc(sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
        if(register_buffer == NULL) {
            printf("Error: malloc failed\n");
            free(register_buffer);
            exit(EXIT_FAILURE);
        }
        memset(register_buffer,0, sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
        memcpy(register_buffer, &sendCode, sizeof(uint8_t));
        memcpy(register_buffer + sizeof(uint8_t), argv[1], sizeof(char[256]));
        memcpy(register_buffer + sizeof(char[256]) + sizeof(uint8_t), argv[4], sizeof(char[32]));

        ssize_t w = write(mbroker_pipe, register_buffer, size);
        free(register_buffer);
        if (w < 0) {                                                                      //maybe remove
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        close(mbroker_pipe);

        int receivingPipe = open(argv[1], O_RDONLY);
        
       
        uint8_t code;
        int32_t returnCode;
        char error[1024];

        ssize_t readBytes = read(receivingPipe, &code, sizeof(uint8_t));
        readBytes += read(receivingPipe, &returnCode, sizeof(int32_t));
        readBytes += read(receivingPipe, error, sizeof(char[1024]));
         if (readBytes == -1) {
            printf("yo\n");
            // ret == -1 signals error
            fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        close(receivingPipe);
        if (returnCode == -1){
            fprintf(stdout,"ERROR %s\n",error);
        } else
        {
            fprintf(stdout, "OK\n");
        }
        return returnCode;
    }
    else if (strcmp(mode, "list"))
    {
        //later
    }
    return 0;
}