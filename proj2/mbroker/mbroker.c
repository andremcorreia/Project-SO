#include "logging.h"
#include "producer-consumer.h"
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
    bool has_publisher;
    int pub_pipe;
    ssize_t bytes_written;

    pthread_cond_t termination;
    pthread_mutex_t termination_lock;

    pthread_cond_t alarm;
    pthread_mutex_t alarm_lock;

    struct mainNode* next;
};

struct linkedList {
    struct mainNode* head;
    struct mainNode* tail;
};

void addBox(struct linkedList* list, char* box_name) {
    struct mainNode* newNode = (struct mainNode*)malloc(sizeof(struct mainNode));
    memcpy(newNode->box_name,box_name,sizeof(char[32]));
    char file[33];
    sprintf(file,"/%s",box_name);
    memcpy(newNode->file_name,file,sizeof(char[33]));
    pthread_cond_init(&newNode->alarm,NULL);
    pthread_mutex_init(&newNode->alarm_lock,NULL);
    pthread_cond_init(&newNode->termination,NULL);
    pthread_mutex_init(&newNode->termination_lock,NULL);
    newNode->bytes_written = 0;
    newNode->pub_pipe = 0;
    newNode->has_publisher = false;
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
    if (list->head == NULL) 
        return;

    struct mainNode* curr = list->head;
    struct mainNode* prev = NULL;

    if (curr->has_publisher)
    {
        close(curr->pub_pipe);
    }
    pthread_mutex_lock(&curr->alarm_lock);
    pthread_cond_destroy(&curr->alarm);
    pthread_mutex_unlock(&curr->alarm_lock);
    pthread_mutex_destroy(&curr->alarm_lock);

    pthread_mutex_lock(&curr->termination_lock);
    pthread_cond_destroy(&curr->termination);
    pthread_mutex_unlock(&curr->termination_lock);
    pthread_mutex_destroy(&curr->termination_lock);
    while (curr != NULL) {
        if (!strcmp(curr->box_name,box_name)) {
            if (prev == NULL) {
                list->head = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr);
            curr = NULL;
            break;
        }
        prev = curr;
        curr = curr->next;
    }
}

struct mainNode* findBox(struct linkedList* list, char* box_name) {
    struct mainNode* curr = list->head;
    while (curr != NULL) {
        if (!strcmp(curr->box_name,box_name)) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t podeProd = PTHREAD_COND_INITIALIZER;
pthread_cond_t podeCons = PTHREAD_COND_INITIALIZER;
pc_queue_t queue;

bool ended = false;

static void sig_handler(int signum){
    if(signum == SIGINT){
        ended = true;
    }
}

void publisher(char* clientPipe,char* boxName){
    struct mainNode* box = findBox(&boxes,boxName);
    if (box == NULL)
        return;
    box->has_publisher = true;

    char file[33];
    sprintf(file,"/%s",boxName);
    int f = tfs_open(file,TFS_O_APPEND);
    if (f == -1){
        printf("error to do in L126 - mbroker.c\n");
        return;
    }
    char msg[1024];

    int receivingPipe = open(clientPipe, O_RDONLY);
    box->pub_pipe = receivingPipe;

    bool active = true;

    while (!ended && active)
    {    
        ssize_t readBytes = read(receivingPipe, msg, sizeof(msg));
        if (readBytes > 0)
        {
            /*ssize_t written = */tfs_write(f, msg, strlen(msg)+1);
            //box->bytes_written += written;
            //pthread_mutex_lock(&box->alarm_lock);
            //pthread_cond_broadcast(&box->alarm);
            printf("%s wrote: %s\n",clientPipe, msg);
            //pthread_mutex_unlock(&box->alarm_lock);
        }
        else{
            active = false;
        } 
    }
    //box->has_publisher = false;
    printf("out com o publisher\n");
    tfs_close(f);
    close(receivingPipe);

}

void subscriber(char* clientPipe,char* boxName){
    struct mainNode* box = findBox(&boxes,boxName);
    void* sub_buffer;
    uint8_t code = 10;
        
    sub_buffer = malloc(sizeof(uint8_t) + sizeof(char[1024]));
    if(sub_buffer == NULL) {
        printf("Error: malloc failed\n");
        free(sub_buffer);
        return;
    }
    int sub_pipe = open(clientPipe, O_WRONLY);
    char buf[1024];
    int f = tfs_open(box->file_name,O_RDONLY);

    ssize_t contents = box->bytes_written;
    ssize_t read = contents;

    tfs_read(f, buf, (size_t)contents);
    memset(sub_buffer,0, sizeof(uint8_t) + sizeof(char[1024]));
    memcpy(sub_buffer, &code, sizeof(uint8_t));
    memcpy(sub_buffer + sizeof(uint8_t), buf, sizeof(char[1024]));
    ssize_t z = write(sub_pipe, sub_buffer, sizeof(uint8_t) + sizeof(char[1024]));
    if (z == -1) {
        close(sub_pipe);
        free(sub_buffer);
        fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
        return;
    }
    
    while (!ended)
    {
        char* buf2 = (char*)malloc(sizeof(char[1024]));
        memset(buf2,0, sizeof(char[1024]));

        pthread_mutex_lock(&box->alarm_lock);
        pthread_cond_wait(&box->alarm, &box->alarm_lock);
        pthread_mutex_unlock(&box->alarm_lock);

        contents = box->bytes_written; // locks needed

        tfs_read(f, buf2, (size_t)(contents - read));
        read = contents;

        memset(sub_buffer,0, sizeof(uint8_t) + sizeof(char[1024]));
        memcpy(sub_buffer, &code, sizeof(uint8_t));
        memcpy(sub_buffer + sizeof(uint8_t), buf2, sizeof(char[1024]));

        ssize_t y = write(sub_pipe, sub_buffer, sizeof(uint8_t) + sizeof(char[1024]));
        if (y == -1) {
            close(sub_pipe);
            free(sub_buffer);   
            fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
            return;
        }
        free(buf2);
    }
    close(sub_pipe);
    free(sub_buffer);
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
    memset(sendBuffer,0, sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
    memcpy(sendBuffer, &code, sizeof(uint8_t));
    memcpy(sendBuffer + sizeof(uint8_t), &return_code, sizeof(uint32_t));
    memcpy(sendBuffer + sizeof(uint32_t) + sizeof(uint8_t), error, sizeof(char[32]));
    ssize_t w = write(managerPipe, sendBuffer, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(char[1024]));
    free(sendBuffer);
    if (w < 0) {                                                                   //maybe remove
        fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
        return;
    }

    close(managerPipe);
}

void manageCreate(char* clientPipe, char* boxName){
    char file[33];
    sprintf(file,"/%s",boxName);
    int f = tfs_open(file, TFS_O_CREAT);
    if (f == -1){
        printf("file cant be opened\n");
    }
    addBox(&boxes,boxName);
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
    tfs_unlink(file);
    removeBox(&boxes, boxName);
    printf("ayo unBoxed\n");
    managerReplier(6, clientPipe);
}

void thread(){
    while (!ended) {
        //pthread_mutex_lock(&mutex);
        //while (count == 0) pthread_cond_wait(&podeCons, &mutex);
        struct request *item = (struct request*)pcq_dequeue(&queue);
        //pthread_cond_signal(&podeProd);
        //pthread_mutex_unlock(&mutex);
        switch (item->code){
        case 1:
            //max 1 per box!
            printf("started %s\n",item->client_pipe);
            publisher(item->client_pipe,item->box);
            break;
        case 2:
            subscriber(item->client_pipe,item->box);
            break;
        case 3:
            manageCreate(item->client_pipe,item->box);
            break;
        case 5:
            manageRemove(item->client_pipe,item->box);
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

    if(signal(SIGINT, sig_handler) == SIG_ERR){
        printf("error sig\n");
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
    while (!ended) { //ver para sigint quando se da ctrl c na mbroker
        uint8_t code = 0;
        ssize_t readBytes = read(receivingPipe, &code, sizeof(uint8_t));
        if (readBytes == 0)
                    break;
        char clientPipe[256];
        char boxName[320];

        //pthread_mutex_lock(&mutex);
        //while (count == Max_sessions) pthread_cond_wait(&podeProd, &mutex);

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
        //pthread_cond_signal(&podeCons);
        //pthread_mutex_unlock(&mutex);
    }
    ended = true;

    for (int i = 0; i < Max_sessions; i++)
    {
        pthread_join(tid[i], NULL);
    }
    
    pcq_destroy(&queue);
    tfs_destroy();
    return 0;
} 
