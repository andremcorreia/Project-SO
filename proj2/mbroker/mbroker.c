#include "logging.h"
#include "producer-consumer.h"

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
    if(newNode == NULL) {
        printf("Error: malloc failed\n");
        return;
    }
    if(newNode == NULL) {
        printf("Error: malloc failed\n");
        return;
    }
    memcpy(newNode->box_name,box_name,sizeof(char[32]));
    newNode->subs = (struct subscriptions*)malloc(sizeof(struct subscriptions));
    if(newNode->subs == NULL) {
        printf("Error: malloc failed\n");
        free(newNode);
        return;
    }
    if(newNode->subs == NULL) {
        printf("Error: malloc failed\n");
        free(newNode);
        return;
    }
    newNode->subs->head = NULL;
    newNode->subs->tail = NULL;
    newNode->next = NULL;
    //pthread_mutex_lock(&list_mutex);
    //pthread_mutex_lock(&list_mutex);
    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
    //pthread_mutex_unlock(&list_mutex);
    free(newNode->subs);
    //pthread_mutex_unlock(&list_mutex);
    free(newNode->subs);
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
    if(newNode == NULL) {
        printf("Error: malloc failed\n");
        free(newNode);
        exit(EXIT_FAILURE);
    }
    if(newNode == NULL) {
        printf("Error: malloc failed\n");
        free(newNode);
        exit(EXIT_FAILURE);
    }
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

struct request
{
    uint8_t code;
    char client_pipe[256];
    char box[32];
};


int Max_sessions;
    
int count=0;
//pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t podeProd = PTHREAD_COND_INITIALIZER;
pthread_cond_t podeCons = PTHREAD_COND_INITIALIZER;
pc_queue_t queue;

bool ended = false;

void publisher(char* clientPipe,char* boxName){
    char file[33];
    sprintf(file,"/%s",boxName);
    int f = tfs_open(file,TFS_O_APPEND);
    if (f == -1){
        printf("error to do in L126 - mbroker.c\n");
        return;
    }
    char msg[1024];

    int receivingPipe = open(clientPipe, O_RDWR); // rdonly dava segmentation 
    int receivingPipe = open(clientPipe, O_RDWR); // rdonly dava segmentation 

    bool active = true;

    while (active)
    {    
        ssize_t readBytes = read(receivingPipe, msg, sizeof(msg));
        if (readBytes !=0)
        {
            //pthread_mutex_lock(&file_mutex);
            //pthread_mutex_lock(&file_mutex);
            tfs_write(f, msg, sizeof(msg));
            //pthread_mutex_unlock(&file_mutex);
            //pthread_mutex_unlock(&file_mutex);
            if (strlen(msg) == sizeof(msg))
                printf("no /o here\n");
            printf("%s wrote: %s\n",clientPipe, msg);
            printf("%s wrote: %s\n",clientPipe, msg);
        }
        else{
            active = false;
        }
        
    }
    tfs_close(f);
    close(receivingPipe);

}

void managerReplier(uint8_t code, char* clientPipe){
    
    
    int managerPipe = open(clientPipe, O_WRONLY);


    char error[1024] = "error placeholder";
    uint32_t return_code = 0; //falta -1 se erro
    void* sendBuffer;


    sendBuffer = malloc(sizeof(uint8_t) + sizeof(uint32_t) + sizeof(char[1024]));
    if(sendBuffer == NULL) {
        printf("Error: malloc failed\n");
        free(sendBuffer);
        exit(EXIT_FAILURE);
    }
    if(sendBuffer == NULL) {
        printf("Error: malloc failed\n");
        free(sendBuffer);
        exit(EXIT_FAILURE);
    }
    memset(sendBuffer,0, sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
    memcpy(sendBuffer, &code, sizeof(uint8_t));
    memcpy(sendBuffer + sizeof(uint8_t), &return_code, sizeof(uint32_t));
    memcpy(sendBuffer + sizeof(uint32_t) + sizeof(uint8_t), error, sizeof(char[32]));
    printf("responding with: %s\n",(char*)sendBuffer);

    ssize_t w = write(managerPipe, sendBuffer, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(char[1024]));
    printf("responding with: a");
    printf("responding with: %s\n",(char*)sendBuffer);

    ssize_t w = write(managerPipe, sendBuffer, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(char[1024]));
    printf("responding with: a");
    free(sendBuffer);


    if (w < 0) {    
        printf("aqui\n");                                                                  //maybe remove
        fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(managerPipe);
}

void manageCreate(char* clientPipe, char* boxName){
    char file[33];
    sprintf(file,"/%s",boxName);
    int f = tfs_open(file, TFS_O_CREAT);
    if (f == -1){
        printf("file cant be opened\n");
        printf("file cant be opened\n");
    }
    addBox(&boxes,file);
    printf("Boxed\n");
    fprintf(stdout, "OK\n");
    tfs_close(f);
    managerReplier(4, clientPipe); 
}

void manageRemove(char* clientPipe, char* boxName){
    char file[33];
    sprintf(file,"/%s",boxName);
    if(tfs_open(file, 0) == -1){
        printf("file cant be opened so it cant be removed\n");
    }
    if(tfs_open(file, 0) == -1){
        printf("file cant be opened so it cant be removed\n");
    }
    tfs_unlink(file);
    removeBox(&boxes, file);
    removeBox(&boxes, file);
    printf("ayo unBoxed\n");
    managerReplier(6, clientPipe);
}

void thread(){
    while (!ended) {
        pthread_mutex_lock(&mutex);
        while (count == 0) pthread_cond_wait(&podeCons, &mutex);
        struct request item = *(struct request*)pcq_dequeue(&queue);
        pthread_cond_signal(&podeProd);
        pthread_mutex_unlock(&mutex);
        switch (item.code)
        {
        case 1:
            //max 1 per box!
            printf("started %s\n",item.client_pipe);
            publisher(item.client_pipe,item.box);
            break;
        case 2:
            //subscriber();
            break;
        case 3:
            manageCreate(item.client_pipe,item.box);
            break;
        case 5:
            manageRemove(item.client_pipe,item.box);
            break;
        case 7:
            //managerList();
            break;
        default:
            break;
        }


        count--;
        // da queue tirar code e args necessarios
        // call a funcao needeed
        
    }

    // inicializer
    //while(1) {
        // consumir
        // codigo trata publisher/sub/manager
        // if/else para ver se e' publisher/sub...
    //}
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    if (argc != 3){
        fprintf(stderr, "usage: mbroker <pipename> <max_sesions>\n");
    }
    Max_sessions = atoi(argv[2]);

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

    pcq_create(&queue, (size_t)Max_sessions);

    pthread_t tid[Max_sessions];
    
    for (int i = 0; i < Max_sessions; i++)
    {
        if (pthread_create(&tid[i], NULL, (void*)thread, (void*) NULL) != 0) {
            printf("Failed while creating the threads\n");
            return -1;
        }
    }

    int receivingPipe = open(argv[1], O_RDWR);
    signal(SIGPIPE, SIG_IGN);
    while (ended) { //ver para sigint quando se da ctrl c na mbroker
    while (ended) { //ver para sigint quando se da ctrl c na mbroker
        uint8_t code = 0;
        ssize_t readBytes = read(receivingPipe, &code, sizeof(uint8_t));
        printf("code: %d\n", code);
        if (readBytes == 0)
                    break;
        char clientPipe[256];
        char boxName[320];

        pthread_mutex_lock(&mutex);
        while (count == Max_sessions) pthread_cond_wait(&podeProd, &mutex);

        struct request item;
        item.code = code;
        readBytes += read(receivingPipe, clientPipe, sizeof(char[256]));
        memcpy(item.client_pipe, clientPipe, sizeof(char[256]));
        if (code != 7)
        {
            readBytes += read(receivingPipe, boxName, sizeof(char[32]));
            memcpy(item.box, boxName, sizeof(char[32]));
        }
        else
        {
            memset(item.box, 0, sizeof(char[32]));
        }
        
        pcq_enqueue(&queue, &item);
        count++;
        pthread_cond_signal(&podeCons);
        pthread_mutex_unlock(&mutex);
    }
    ended = true;

    for (int i = 0; i < Max_sessions; i++)
    {
        pthread_join(tid[i], NULL);
    }
    return -1;
    
    pcq_destroy(&queue);
    tfs_destroy();
    
} 
