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

struct box {
    char box_name[32];
    uint64_t box_size;
    uint64_t n_publishers;
    uint64_t n_subscribers;
};

int compare(const void* a, const void* b) {
    struct box* boxA = (struct box*) a;
    struct box* boxB = (struct box*) b;
    return strcmp(boxA->box_name, boxB->box_name);
}

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
    else if (!strcmp(mode, "list"))
    {
        uint8_t sendCode = 7;
        int mbroker_pipe = open(argv[2], O_WRONLY);

        void* register_buffer;
        
        register_buffer = malloc(sizeof(uint8_t) + sizeof(char[256]));
        if(register_buffer == NULL) {
            printf("Error: malloc failed\n");
            free(register_buffer);
            exit(EXIT_FAILURE);
        }
        memset(register_buffer,0, sizeof(uint8_t) + sizeof(char[256]));
        memcpy(register_buffer, &sendCode, sizeof(uint8_t));
        memcpy(register_buffer + sizeof(uint8_t), argv[1], sizeof(char[256]));

        ssize_t w = write(mbroker_pipe, register_buffer, sizeof(uint8_t) + sizeof(char[256]));
        free(register_buffer);
        if (w < 0) {                                                                      //maybe remove
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        close(mbroker_pipe);

        uint8_t last = (uint8_t)0;
        int receivingPipe = open(argv[1], O_RDONLY);
        struct box* boxes = NULL;
        int counter = 0;
        while (!last)
        {
            uint8_t code;
            char box_name[32];
            uint64_t box_size;
            uint64_t n_publishers;
            uint64_t n_subscribers;

            ssize_t readBytes = read(receivingPipe, &code, sizeof(uint8_t));
            readBytes += read(receivingPipe, &last, sizeof(uint8_t));
            readBytes += read(receivingPipe, box_name, sizeof(char[32]));
            readBytes += read(receivingPipe, &box_size, sizeof(uint64_t));
            readBytes += read(receivingPipe, &n_publishers, sizeof(uint64_t));
            readBytes += read(receivingPipe, &n_subscribers, sizeof(uint64_t));

            if (last && strlen(box_name) == 0)
            {
                fprintf(stdout,"NO BOXES FOUND\n");
            }
            else
            {
                struct box newBox;
                memcpy(newBox.box_name, box_name, sizeof(char[32]));
                newBox.box_size = box_size;
                newBox.n_publishers = n_publishers;
                newBox.n_subscribers = n_subscribers;
                counter++;
                boxes = realloc(boxes, (size_t)counter * sizeof(struct box));
                boxes[counter - 1] = newBox;
            }
        }
        qsort(boxes, (size_t)counter, sizeof(struct box), compare);

        for (int i = 0; i < counter; i++) {
            fprintf(stdout, "%s %zu %zu %zu\n", boxes[i].box_name, boxes[i].box_size, boxes[i].n_publishers, boxes[i].n_subscribers);
        }
        close(receivingPipe);
        free(boxes);
        
    }
    return 0;
}