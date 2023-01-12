#include "producer-consumer.h"

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

int pcq_create(pc_queue_t *queue, size_t capacity){
    
    queue->pcq_buffer = (void**)malloc(capacity * sizeof(void*));
    if (queue->pcq_buffer == NULL) {
        free(queue);
        return NULL;
    }
    queue->pcq_capacity = capacity;
    queue->pcq_current_size = 0;
    queue->pcq_head = NULL;
    queue->pcq_tail = NULL;
    pthread_mutex_init(&queue->pcq_current_size_lock,NULL);
    pthread_mutex_init(&queue->pcq_head_lock,NULL);
    pthread_mutex_init(&queue->pcq_tail_lock,NULL);
    pthread_mutex_init(&queue->pcq_pusher_condvar_lock,NULL);
    pthread_mutex_init(&queue->pcq_popper_condvar_lock,NULL);
    return 0;
}

int pcq_destroy(pc_queue_t *queue){

    free(queue->pcq_buffer);

    pthread_mutex_destroy(&queue->pcq_current_size_lock);
    pthread_mutex_destroy(&queue->pcq_head_lock);
    pthread_mutex_destroy(&queue->pcq_tail_lock);
    pthread_mutex_destroy(&queue->pcq_pusher_condvar_lock);
    pthread_cond_destroy(&queue->pcq_pusher_condvar);
    pthread_mutex_destroy(&queue->pcq_popper_condvar_lock);
    pthread_cond_destroy(&queue->pcq_popper_condvar);
    return 0;
}

int pcq_enqueue(pc_queue_t *queue, void *elem){

}

void *pcq_dequeue(pc_queue_t *queue){

}