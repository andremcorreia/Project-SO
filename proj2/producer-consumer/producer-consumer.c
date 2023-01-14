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
#include <stdlib.h>

bool pcq_is_empty(pc_queue_t *queue) {
    bool is_empty;
    pthread_mutex_lock(&queue->pcq_current_size_lock);
    is_empty = (queue->pcq_current_size == 0);
    pthread_mutex_unlock(&queue->pcq_current_size_lock);
    return is_empty;
}
bool pcq_is_full(pc_queue_t *queue) {
    bool is_full;
    pthread_mutex_lock(&queue->pcq_current_size_lock);
    is_full = (queue->pcq_current_size == queue->pcq_capacity);
    pthread_mutex_unlock(&queue->pcq_current_size_lock);
    return is_full;
}

int pcq_create(pc_queue_t *queue, size_t capacity){

    queue->pcq_buffer = (void**)malloc(capacity * sizeof(void*));
    if (queue->pcq_buffer == NULL) {
        free(queue);
        exit(EXIT_FAILURE);
    }
    queue->pcq_capacity = capacity;
    queue->pcq_current_size = 0;
    queue->pcq_head = 0;
    queue->pcq_tail = 0;
    pthread_mutex_init(&queue->pcq_current_size_lock,NULL);
    pthread_mutex_init(&queue->pcq_head_lock,NULL);
    pthread_mutex_init(&queue->pcq_tail_lock,NULL);
    pthread_mutex_init(&queue->pcq_pusher_condvar_lock,NULL);
    pthread_mutex_init(&queue->pcq_popper_condvar_lock,NULL);
    pthread_cond_init(&queue->pcq_pusher_condvar,NULL);
    pthread_cond_init(&queue->pcq_popper_condvar,NULL);
    return 0;
}

int pcq_destroy(pc_queue_t *queue){

    free(queue->pcq_buffer);

    pthread_mutex_destroy(&queue->pcq_current_size_lock);
    pthread_mutex_destroy(&queue->pcq_head_lock);
    pthread_mutex_destroy(&queue->pcq_tail_lock);
    pthread_mutex_destroy(&queue->pcq_pusher_condvar_lock);
    pthread_mutex_destroy(&queue->pcq_popper_condvar_lock);
    pthread_cond_destroy(&queue->pcq_pusher_condvar);
    pthread_cond_destroy(&queue->pcq_popper_condvar);
    return 0;
}

int pcq_enqueue(pc_queue_t *queue, void *elem){
    int rc = 0;
    if (pcq_is_full(queue)) {
        return -1;
    }
    pthread_mutex_lock(&queue->pcq_pusher_condvar_lock);
    while (queue->pcq_current_size == queue->pcq_capacity) {
        rc = pthread_cond_wait(&queue->pcq_pusher_condvar, &queue->pcq_pusher_condvar_lock);
        if (rc != 0) {
            pthread_mutex_unlock(&queue->pcq_pusher_condvar_lock);
            return rc;
        }
    }

    pthread_mutex_lock(&queue->pcq_head_lock);
    queue->pcq_buffer[queue->pcq_head] = elem;
    queue->pcq_head = (queue->pcq_head + 1) % queue->pcq_capacity;
    pthread_mutex_unlock(&queue->pcq_head_lock);

    pthread_mutex_lock(&queue->pcq_current_size_lock);
    queue->pcq_current_size++;
    pthread_mutex_unlock(&queue->pcq_current_size_lock);

    pthread_cond_signal(&queue->pcq_popper_condvar);
    pthread_mutex_unlock(&queue->pcq_pusher_condvar_lock);

    return 0;
}

void *pcq_dequeue(pc_queue_t *queue){
    int rc = 0;
    void *elem = NULL;
    pthread_mutex_lock(&queue->pcq_popper_condvar_lock);
    while (queue->pcq_current_size == 0) {
        rc = pthread_cond_wait(&queue->pcq_popper_condvar, &queue->pcq_popper_condvar_lock);
        if (rc != 0) {
            pthread_mutex_unlock(&queue->pcq_popper_condvar_lock);
            return NULL;
        }
    }

    pthread_mutex_lock(&queue->pcq_tail_lock);
    elem = queue->pcq_buffer[queue->pcq_tail];
    queue->pcq_tail = (queue->pcq_tail + 1) % queue->pcq_capacity;
    pthread_mutex_unlock(&queue->pcq_tail_lock);

    pthread_mutex_lock(&queue->pcq_current_size_lock);
    queue->pcq_current_size--;
    pthread_mutex_unlock(&queue->pcq_current_size_lock);

    pthread_cond_signal(&queue->pcq_pusher_condvar);
    pthread_mutex_unlock(&queue->pcq_popper_condvar_lock);

    return elem;
}
