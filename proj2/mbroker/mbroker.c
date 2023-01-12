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

struct mainNode {
    char box_name[32];
    char file_name[33];
    struct subscriptions* subs;
    struct mainNode* next;
};

struct subNode {
    char pipe_to_sub[256];
    struct subNode* next;
};

struct subscriptions {
    struct subNode* head;
    struct subNode* tail;
};

struct linkedList {
    struct mainNode* head;
    struct mainNode* tail;
};

void addBox(struct linkedList* list, char* box_name) {
    struct mainNode* newNode = (struct mainNode*)malloc(sizeof(struct mainNode));
    memcpy(newNode->box_name,box_name,sizeof(char[32]));
    newNode->subs = (struct subscriptions*)malloc(sizeof(struct subscriptions));
    newNode->subs->head = NULL;
    newNode->subs->tail = NULL;
    newNode->next = NULL;
    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
}

void removeBox(struct linkedList* list, char* box_name) {
    struct mainNode* curr = list->head;
    struct mainNode* prev = NULL;

    while (curr != NULL) {
        if (!strcmp(curr->box_name,box_name)) {
            if (prev == NULL) {
                list->head = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr->subs);
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }
}

void addSub(struct subscriptions* list, char* pipe_to_sub) {
    struct subNode* newNode = (struct subNode*)malloc(sizeof(struct subNode));
    memcpy(newNode->pipe_to_sub,pipe_to_sub,sizeof(char[256]));
    newNode->next = NULL;
    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
}

void removeSub(struct subscriptions* list, char* pipe_to_sub) {
    struct subNode* curr = list->head;
    struct subNode* prev = NULL;

    while (curr != NULL) {
        if (!strcmp(curr->pipe_to_sub,pipe_to_sub)) {
            if (prev == NULL) {
                list->head = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }
}

struct linkedList boxes;

int Max_sessions;
    
int prodptr=0, consptr=0, count=0;
pthread_mutex_t mutex;
pthread_cond_t podeProd, podeCons; 

void thread(char* clientPipe,char* boxName){
    int buf[Max_sessions]
    // inicializer
    while(1) {
        // consumir
        // codigo trata publisher/sub/manager
        // if/else para ver se e' publisher/sub...
    }
}

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
    Max_sessions = argv[2];

    boxes.head = NULL;
    boxes.tail = NULL;

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
                char file[33];
                sprintf(file,"/%s",boxName);
                int f = tfs_open(file, TFS_O_CREAT);
                if (f == -1){
                    printf("error to do 102 mbroke\n");
                }
                addBox(&boxes,file);
                printf("Boxed\n");
                fprintf(stdout, "OK\n");
                tfs_close(f);
                int managerPipe = open(clientPipe, O_WRONLY);
                char error[1024] = "error placeholder";
                code = 4;
                uint32_t return_code = 0; //falta -1 se erro
                void* sendBuffer;
                sendBuffer = malloc(sizeof(uint8_t) + sizeof(uint32_t) + sizeof(char[1024]));
                memset(sendBuffer,0, sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
                memcpy(sendBuffer, &code, sizeof(uint8_t));
                memcpy(sendBuffer + sizeof(uint8_t), &return_code, sizeof(uint32_t));
                memcpy(sendBuffer + sizeof(uint32_t) + sizeof(uint8_t), error, sizeof(char[32]));
                //printf("responding with: %s\n",sendBuffer);
                ssize_t w = write(managerPipe, sendBuffer, 1024);
                free(sendBuffer);
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
                char file[33];
                sprintf(file,"/%s",boxName);
                tfs_unlink(file);
                printf("ayo unBoxed\n");
                int managerPipe1 = open(clientPipe, O_WRONLY);
                char error1[1024] = "error placeholder";
                code = 6;
                uint32_t return_code1 = 0; //falta -1 se erro
                void* sendBuffer1;
                sendBuffer1 = malloc(sizeof(uint8_t) + sizeof(uint32_t) + sizeof(char[1024]));
                memset(sendBuffer1,0, sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
                memcpy(sendBuffer1, &code, sizeof(uint8_t));
                memcpy(sendBuffer1 + sizeof(uint8_t), &return_code1, sizeof(uint32_t));
                memcpy(sendBuffer1 + sizeof(uint32_t) + sizeof(uint8_t), error1, sizeof(char[32]));
                //printf("responding with: %s\n",sendBuffer1);
                ssize_t r = write(managerPipe1, sendBuffer1, 1024);
                free(sendBuffer1);
                if (r < 0) {    
                    printf("aqui\n");                                                                  //maybe remove
                    fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                close(managerPipe1);
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