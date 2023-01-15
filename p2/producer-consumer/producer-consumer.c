#include <pthread.h>
#include <stdlib.h>
#include "producer-consumer.h"

int pcq_create(pc_queue_t *queue, size_t capacity) {
    queue->pcq_capacity = capacity;
    queue->pcq_current_size = 0;

    queue->pcq_buffer = malloc(sizeof(char) * capacity);
    
    for (int i = 0; i < capacity; i++) {
        queue->pcq_buffer[i] = malloc(sizeof(char) * 1200);
        queue->pcq_buffer[i] = NULL;
    }
    queue->pcq_head = 0;
    queue->pcq_tail = 0;
    pthread_cond_init(&queue->pcq_pusher_condvar, NULL);
    pthread_mutex_init(&queue->pcq_pusher_condvar_lock, NULL);
    // pthread_cond_init(&queue->pcq_p)
    // queue->pcq_pusher_condvar;
    // queue->pcq_pushe
    // queue->pcq_pusher_condvar
    // queue->pcq_pusher_condvar

    return 0;
}

int pcq_destroy(pc_queue_t *queue) {
    for (int i = 0; i < queue->pcq_capacity; i++) {
        free(queue->pcq_buffer[i]);
    }
    free(queue->pcq_buffer);
    free(queue);
    return 0;
}

int pcq_enqueue(pc_queue_t *queue, void *elem) {
    (char*) elem;

    if (pthread_mutex_lock(&queue->pcq_pusher_condvar_lock) != 0) {
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_lock(&queue->pcq_popper_condvar_lock) != 0) {
        exit(EXIT_FAILURE);
    }

    while (queue->pcq_current_size == queue->pcq_capacity) {
        pthread_cond_wait(&queue->pcq_pusher_condvar, 
        &queue->pcq_pusher_condvar_lock);
    }
    queue->pcq_buffer[queue->pcq_tail++] = elem;
    pthread_cond_signal(&queue->pcq_popper_condvar);
    
    if (queue->pcq_tail == queue->pcq_capacity) {
        queue->pcq_tail = 0;
    }

    if (pthread_mutex_unlock(&queue->pcq_pusher_condvar_lock) != 0)
        exit(EXIT_FAILURE);
    if (pthread_mutex_lock(&queue->pcq_popper_condvar_lock) != 0)
        exit(EXIT_FAILURE);

    return queue->pcq_head;
}

void *pcq_dequeue(pc_queue_t *queue) {
    if (pthread_mutex_lock(&queue->pcq_pusher_condvar_lock) != 0) {
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_lock(&queue->pcq_popper_condvar_lock) != 0) {
        exit(EXIT_FAILURE);
    }

     while(queue->pcq_current_size == 0) {
         pthread_cond_wait(&queue->pcq_popper_condvar,
         &queue->pcq_popper_condvar_lock);
     }
    
    queue->pcq_buffer[queue->pcq_head++] = NULL;
    pthread_cond_signal(&queue->pcq_pusher_condvar);

    if (queue->pcq_head == queue->pcq_capacity) {
        queue->pcq_head = 0;
    }

    if (pthread_mutex_unlock(&queue->pcq_pusher_condvar_lock) != 0)
        exit(EXIT_FAILURE);
    if (pthread_mutex_lock(&queue->pcq_popper_condvar_lock) != 0) 
        exit(EXIT_FAILURE);     
}


