#include "producer-consumer.h"
#include "logging.h"

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

#include <stdio.h>

bool pcq_is_empty(pc_queue_t *queue) {
    bool is_empty;
    
    if (pthread_mutex_lock(&queue->pcq_current_size_lock) == -1) {
        WARN("failed to lock mutex: %s", strerror(errno));
        return -1;
    }
    is_empty = (queue->pcq_current_size == 0);
    if (pthread_mutex_unlock(&queue->pcq_current_size_lock) == -1) {
        WARN("failed to unlock mutex: %s", strerror(errno));
        return -1;
    }    
    return is_empty;
}
bool pcq_is_full(pc_queue_t *queue) {
    bool is_full;
    
    if (pthread_mutex_lock(&queue->pcq_current_size_lock) == -1) {
        WARN("failed to lock mutex: %s", strerror(errno));
        return -1;
    }    
    is_full = (queue->pcq_current_size == queue->pcq_capacity);
    if (pthread_mutex_unlock(&queue->pcq_current_size_lock) == -1) {
        WARN("failed to unlock mutex: %s", strerror(errno));
        return -1;
    }
    
    return is_full;
}

int pcq_create(pc_queue_t *queue, size_t capacity){

    queue->pcq_buffer = (void**)malloc(capacity * sizeof(void*));
    if (queue->pcq_buffer == NULL) {
        printf("Error: malloc failed in queue\n");
        free(queue);
        
        exit(EXIT_FAILURE);
    }
    
    queue->pcq_capacity = capacity;
    queue->pcq_current_size = 0;
    queue->pcq_head = 0;
    queue->pcq_tail = 0;
    
    pthread_mutex_init(&queue->pcq_current_size_lock, NULL);
    pthread_mutex_init(&queue->pcq_head_lock, NULL);
    pthread_mutex_init(&queue->pcq_tail_lock, NULL);
    pthread_mutex_init(&queue->pcq_pusher_condvar_lock, NULL);
    pthread_mutex_init(&queue->pcq_popper_condvar_lock, NULL);
    
    pthread_cond_init(&queue->pcq_pusher_condvar, NULL);
    pthread_cond_init(&queue->pcq_popper_condvar, NULL);
    
    return 0;
}

int pcq_destroy(pc_queue_t *queue){
    free(queue->pcq_buffer);
    
    pthread_cond_broadcast(&queue->pcq_popper_condvar);sleep(1); //signal for threads to leave deqeue

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
    
    if (pthread_mutex_lock(&queue->pcq_pusher_condvar_lock) == -1) {
        WARN("failed to lock mutex: %s", strerror(errno));
        return -1;
    }
    while (queue->pcq_current_size == queue->pcq_capacity) {
        rc = pthread_cond_wait(&queue->pcq_pusher_condvar, &queue->pcq_pusher_condvar_lock);
        if (rc != 0) {
            if (pthread_mutex_unlock(&queue->pcq_pusher_condvar_lock) == -1) {
                WARN("failed to unlock mutex: %s", strerror(errno));
                return -1;
            }    
            return rc;
        }
    }

    if (pthread_mutex_lock(&queue->pcq_head_lock) == -1) {
        WARN("failed to lock mutex: %s", strerror(errno));
        return -1;
    }
    
    queue->pcq_buffer[queue->pcq_head] = elem;
    queue->pcq_head = (queue->pcq_head + 1) % queue->pcq_capacity;
    if (pthread_mutex_unlock(&queue->pcq_head_lock) == -1) {
        WARN("failed to unlock mutex: %s", strerror(errno));
        return -1;
    }

    if (pthread_mutex_lock(&queue->pcq_current_size_lock) == -1) {
        WARN("failed to lock mutex: %s", strerror(errno));
        return -1;
    }
    
    queue->pcq_current_size++;
    if (pthread_mutex_unlock(&queue->pcq_current_size_lock) == -1) {
        WARN("failed to unlock mutex: %s", strerror(errno));
        return -1;
    }

    pthread_cond_signal(&queue->pcq_popper_condvar);
    if (pthread_mutex_unlock(&queue->pcq_pusher_condvar_lock) == -1) {
        WARN("failed to unlock mutex: %s", strerror(errno));
        return -1;
    }
    return 0;
}

void *pcq_dequeue(pc_queue_t *queue){
    int rc = 0;
    void *elem = NULL;
    void *copy = NULL;

    pthread_mutex_lock(&queue->pcq_popper_condvar_lock);
    
    while (queue->pcq_current_size == 0) {
        rc = pthread_cond_wait(&queue->pcq_popper_condvar, &queue->pcq_popper_condvar_lock);
        
        if (rc != 0 || pcq_is_empty(queue)) {
        
            if (pthread_mutex_unlock(&queue->pcq_popper_condvar_lock) == -1) {
                WARN("failed to unlock mutex: %s", strerror(errno));
                return NULL;
            }        
            return NULL;
        }
    }

    if (pthread_mutex_lock(&queue->pcq_tail_lock) == -1) {
        WARN("failed to lock mutex: %s", strerror(errno));
        return NULL;
    }
    
    elem = queue->pcq_buffer[queue->pcq_tail];
    queue->pcq_tail = (queue->pcq_tail + 1) % queue->pcq_capacity;
    if (pthread_mutex_unlock(&queue->pcq_tail_lock) == -1) {
        WARN("failed to unlock mutex: %s", strerror(errno));
        return NULL;
    }

    if (pthread_mutex_lock(&queue->pcq_current_size_lock) == -1) {
        WARN("failed to lock mutex: %s", strerror(errno));
        return NULL;
    }
    
    queue->pcq_current_size--;
    if (pthread_mutex_unlock(&queue->pcq_current_size_lock) == -1) {
        WARN("failed to unlock mutex: %s", strerror(errno));
        return NULL;
    }
    
    pthread_cond_signal(&queue->pcq_pusher_condvar);
    if (pthread_mutex_unlock(&queue->pcq_popper_condvar_lock) == -1) {
        WARN("failed to unlock mutex: %s", strerror(errno));
        return NULL;
    }

    copy = malloc(sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
    	
    if(copy == NULL){ 
        printf("Error: malloc failed in queue\n");
        free(copy);
        return NULL;
    }
    
    memcpy(copy, elem, sizeof(uint8_t) + sizeof(char[256]) + sizeof(char[32]));
    
    return copy;
}