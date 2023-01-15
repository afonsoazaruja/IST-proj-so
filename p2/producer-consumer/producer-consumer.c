#include <stdlib.h>
#include "producer-consumer.h"


int pcq_create(pc_queue_t *queue, size_t capacity) {
    queue->pcq_capacity = capacity;
    queue->pcq_current_size = 0;

    queue->pcq_buffer = malloc(sizeof(char) * capacity);
    return 0;
}

// int pcq_destroy(pc_queue_t *queue) {

// }

// int pcq_enqueue(pc_queue_t *queue, void *elem) {

// }

// void *pcq_dequeue(pc_queue_t *queue) {

//}


