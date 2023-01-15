#include <stdlib.h>
#include "producer-consumer.h"


// int pcq_create(pc_queue_t *queue, size_t capacity) {
//     queue->pcq_capacity = capacity;
//     queue->pcq_current_size = 0;

//     queue->pcq_buffer = malloc(sizeof(char) * capacity);
    
//     for (int i = 0; i < capacity; i++) {
//         queue->pcq_buffer[i] = malloc(sizeof(char) * 1200);
//     }

//     return 0;
// }

// int pcq_destroy(pc_queue_t *queue) {
//     for (int i = 0; i < queue->pcq_capacity; i++) {
//         free(queue->pcq_buffer[i]);
//     }
//     free(queue->pcq_buffer);
//     free(queue);
//     return 0;
// }

// int pcq_enqueue(pc_queue_t *queue, void *elem) {
//     (char*) elem;
//     // queue->
    
//     return 0;
// }

// void *pcq_dequeue(pc_queue_t *queue) {

//}


