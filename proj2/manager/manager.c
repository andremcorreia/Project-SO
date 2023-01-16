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

void cleanPipe(char* pipe){
    if (unlink(pipe) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", pipe, strerror(errno));
        exit(EXIT_FAILURE);
    }
}
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
    
    if (argc !=4 && argc != 5){
        fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> <pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> <pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> <pipe_name> list\n");
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);

    char* mode = argv[3];
    size_t size = sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]);

    cleanPipe(argv[1]);

    // Create pipe
    if (mkfifo(argv[1], 0666) != 0) { // permissions changed to allow read and write for all users
        fprintf(stderr, "[ERR]: manager mkfifo failed: %s\n", strerror(errno));
        
        exit(EXIT_FAILURE);
    }

    if (!strcmp(mode, "create") || !strcmp(mode, "remove")) {
        uint8_t send_code = 3;
        
        if (!strcmp(mode, "remove"))
            send_code = 5;
        
        int mbroker_pipe = open(argv[2], O_WRONLY);
        if (mbroker_pipe == -1) {
            close(mbroker_pipe);
            printf("ERROR: No mbroker\n");
            cleanPipe(argv[1]);
        
            exit(EXIT_FAILURE);
        }

        void* register_buffer;
        register_buffer = malloc(sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
        if(register_buffer == NULL) {
            printf("Error: malloc failed in manager\n");
            free(register_buffer);
            cleanPipe(argv[1]);
        
            exit(EXIT_FAILURE);
        }
        
        memset(register_buffer,0, sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
        
        memcpy(register_buffer, &send_code, sizeof(uint8_t));
        memcpy(register_buffer + sizeof(uint8_t), argv[1], sizeof(char[256]));
        memcpy(register_buffer + sizeof(char[256]) + sizeof(uint8_t), argv[4], sizeof(char[32]));

        ssize_t w = write(mbroker_pipe, register_buffer, size);
        free(register_buffer);
        if (w == -1) {                                                                      //maybe remove
            printf("Failed to register in mbroker\n");
            close(mbroker_pipe);
            cleanPipe(argv[1]);
            
            exit(EXIT_FAILURE);
        }
        close(mbroker_pipe);
        
        int self_pipe = open(argv[1], O_RDONLY);
        
        uint8_t code;
        int32_t return_code;
        char error[1024];
        ssize_t readBytes = read(self_pipe, &code, sizeof(uint8_t));
        readBytes += read(self_pipe, &return_code, sizeof(int32_t));
        readBytes += read(self_pipe, error, sizeof(char[1024]));
        
        if (readBytes == -1) {
            fprintf(stderr, "[ERR]: read failed in manager: %s\n", strerror(errno));
            close(self_pipe);
            cleanPipe(argv[1]);
            
            exit(EXIT_FAILURE);
        }
        close(self_pipe);
        
        if (return_code == -1) {
            fprintf(stdout,"ERROR %s\n",error);
        } else {
            fprintf(stdout, "OK\n");
        }
        return return_code;
    }
    else if (!strcmp(mode, "list")) {
        uint8_t send_code = 7;
        int mbroker_pipe = open(argv[2], O_WRONLY);

        void* register_buffer;
        register_buffer = malloc(sizeof(uint8_t) + sizeof(char[256]));
        if(register_buffer == NULL) {
            printf("Error: malloc failed in manager\n");
            free(register_buffer);
            close(mbroker_pipe);
            cleanPipe(argv[1]);
            
            exit(EXIT_FAILURE);
        
        }

        memset(register_buffer,0, sizeof(uint8_t) + sizeof(char[256]));
        
        memcpy(register_buffer, &send_code, sizeof(uint8_t));
        memcpy(register_buffer + sizeof(uint8_t), argv[1], sizeof(char[256]));

        ssize_t w = write(mbroker_pipe, register_buffer, sizeof(uint8_t) + sizeof(char[256]));
        free(register_buffer);
        if (w < 0) {                                                                      //maybe remove
            printf("Failed to register in mbroker\n");
            close(mbroker_pipe);
            cleanPipe(argv[1]);
            
            exit(EXIT_FAILURE);
        }
        close(mbroker_pipe);

        int counter = 0;
        int self_pipe = open(argv[1], O_RDONLY);
        uint8_t last = 0;
        struct box* boxes = NULL;
        while (!last) {
            uint8_t code;
            char box_name[32];
            uint64_t box_size;
            uint64_t n_publishers;
            uint64_t n_subscribers;

            ssize_t readBytes = read(self_pipe, &code, sizeof(uint8_t));
            readBytes += read(self_pipe, &last, sizeof(uint8_t));
            readBytes += read(self_pipe, box_name, sizeof(char[32]));
            readBytes += read(self_pipe, &box_size, sizeof(uint64_t));
            readBytes += read(self_pipe, &n_publishers, sizeof(uint64_t));
            readBytes += read(self_pipe, &n_subscribers, sizeof(uint64_t));

            if (last && strlen(box_name) == 0) {
                fprintf(stdout,"NO BOXES FOUND\n");
            }
            else {
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
        close(self_pipe);
        
        free(boxes);
        
        cleanPipe(argv[1]);
        
    }
    return 0;
}