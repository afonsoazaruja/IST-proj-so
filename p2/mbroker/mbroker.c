#include "../utils/logging.h"
#include "boxes.h"
#include "../producer-consumer/producer-consumer.h"

#define BUFFER_SIZE 1200
#define MESSAGE_SIZE 1024
#define MAX_BOXES 64

int fserv;
pthread_mutex_t mutex_boxes[MAX_BOXES];
pthread_cond_t cond_boxes[MAX_BOXES];
int *threads_availability;

void uint64_to_bytes(uint64_t value, char bytes[], int index) {
    for (int i = 0; i < 8; i++) {
        bytes[index + i] = (char) ((value >> (56 - (i * 8))) & 0xFF);   
    }
}

void int32_to_bytes(int32_t value, char bytes[], int index) {
    for (int i = 0; i < 4; i++) {
        bytes[index + i] = (char) ((value >> (24 - (i * 8))) & 0xFF);   
    }
}

void send_response(uint8_t op_code, int32_t ret_code, int pipe) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = (char) op_code;
    switch(op_code) {
        case 4: // create box response
        case 6: // remove box response
            int32_to_bytes(ret_code, buffer, 1);
            memcpy(buffer+5, err_msg, strlen(err_msg));
            safe_write(pipe, buffer, BUFFER_SIZE);
            break;
        case 8: // list boxes
            if (num_of_boxes == 0) {
                buffer[1] = 1;
                safe_write(pipe, buffer, BUFFER_SIZE);
                break;
            }
            int n_boxes = num_of_boxes;
            for (int i = 0; i < MAX_BOXES; i++) {
                if (system_boxes[i]->state == TAKEN) {
                    memset(buffer, 0, BUFFER_SIZE);
                    buffer[0] = (char) op_code;
                    buffer[1] = (char) 0;
                    if (--n_boxes == 0) buffer[1] = (char) 1;
                    memcpy(buffer+2, system_boxes[i]->box_name, 32);
                    uint64_to_bytes(system_boxes[i]->box_size, buffer, 34); 
                    uint64_to_bytes(system_boxes[i]->n_publishers, buffer, 42); 
                    uint64_to_bytes(system_boxes[i]->n_subscribers, buffer, 50); 
                    safe_write(pipe, buffer, BUFFER_SIZE);
                }
            }
            break;

        default: break;
    }
}

void read_pipe_name(char pipe_name[]) {
    safe_read(fserv, pipe_name, 256);
}
void read_box_name(char box_name[]) {
    safe_read(fserv, box_name, 32);
}
void read_pipe_and_box_name(char pipe_name[], char box_name[]) {
    read_pipe_name(pipe_name);
    read_box_name(box_name);
}

void request_handler(char *op_code) {
    int fcli;
    ssize_t ret;
    char buffer[BUFFER_SIZE];
    char message[MESSAGE_SIZE];
    char pipe_name[256];
    char box_name[32];

    memset(buffer, 0, BUFFER_SIZE);
    memset(pipe_name, 0, 256);
    memset(box_name, 0, 32);
    long index;

    switch((uint8_t)op_code[0]) {
        case 1: // create publisher
            read_pipe_and_box_name(pipe_name, box_name);
            fcli = open_pipe(pipe_name, O_RDONLY);
            index = find_box(box_name);
            if (index == -1 || system_boxes[index]->n_publishers == 1) {
                close(fcli);
                break;
            }
            ret = safe_read(fcli, buffer, BUFFER_SIZE);
            memset(buffer, 0, BUFFER_SIZE);
            system_boxes[index]->n_publishers = 1;
            int fd = tfs_open(box_name, TFS_O_APPEND);
            if (fd == -1) exit(EXIT_FAILURE);
            size_t len = 1;
            while(true) {
                ret = read(fcli, buffer, MESSAGE_SIZE);
                if (ret <= 0) break;
                len = strlen(buffer);
                system_boxes[index]->box_size += len;
                buffer[len-1] = '\0';

                ret = tfs_write(fd, buffer, len);
                if (ret == -1) exit(EXIT_FAILURE);
                memset(buffer, 0, BUFFER_SIZE);
            }
            // if read returns 0, the pipe was close so the publisher disconnects
            tfs_close(fd);
            system_boxes[index]->n_publishers = 0;
            close(fcli);
            break;

        case 2: // create subscriber
            read_pipe_and_box_name(pipe_name, box_name);
            fcli = open_pipe(pipe_name, O_WRONLY);
            index = find_box(box_name);
            if (index == -1) {
                close(fcli);
                break;
            }
            system_boxes[index]->n_subscribers++;
            fd = tfs_open(box_name, TFS_O_CREAT);
            len = system_boxes[index]->box_size;
            while(len > 0) {
                memset(message, 0, 1024);
                memset(buffer, 0, BUFFER_SIZE);
                size_t offset = 0;
                ret = tfs_read(fd, buffer, 1);
                while(buffer[0] != 0) {
                    memcpy(message+offset, buffer, 1);
                    len--; offset++;
                    ret = tfs_read(fd, buffer, 1);
                }
                message[offset] = '\0';
                ret = write(fcli, message, offset);
                len--;
            }

            tfs_close(fd);
            // while(true) {

            // }

            system_boxes[index]->n_subscribers--;
            close(fcli);
            break;
    
        case 3: // create box
            read_pipe_and_box_name(pipe_name, box_name);
            fcli = open_pipe(pipe_name, O_WRONLY);
            send_response(4, create_box(box_name), fcli);
            close(fcli);
            break;
        
        case 5: // remove box
            read_pipe_and_box_name(pipe_name, box_name);
            fcli = open_pipe(pipe_name, O_WRONLY);
            send_response(6, remove_box(box_name), fcli);
            close(fcli);
            break;
        
        case 7: // list
            read_pipe_name(pipe_name);
            fcli = open_pipe(pipe_name, O_WRONLY);
            send_response(8, 0, fcli);
            close(fcli);
            break;
            
        default: return;
    }
    return;
}
 
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: mbroker <register_pipe_name> <max_sessions>\n");
        return -1;
    }

    char *register_pipe_name = argv[1];
    size_t num_threads = (size_t) atoi(argv[2]);
    // pthread_t threads[num_threads];

    tfs_params params = tfs_default_params();
    params.max_open_files_count = num_threads;
    params.max_inode_count = MAX_BOXES;
    if (tfs_init(&params) == -1) {
        return -1;
    }

    init_boxes();
    threads_availability = malloc(sizeof(int) * ((size_t) (num_threads)));

    pc_queue_t *queue = malloc(sizeof(pc_queue_t));

    pcq_create(queue, num_threads);

    // initialize mutexes
    if (pthread_mutex_init(mutex_boxes, NULL) != 0) {
        perror("Failed to init Mutex");
        exit(EXIT_FAILURE);
    }

    // initialize conditional variables
    if (pthread_cond_init(cond_boxes, NULL) != 0) {
        perror("Failed to init Mutex");
        exit(EXIT_FAILURE);
    }

    // // create threads
    // for (int i = 0; i < num_threads; i++) {
    //     if (pthread_create(&threads[i], NULL, request_handler, NULL) != 0) {
    //         printf("ERROR Creating thread.\n");
    //         return -1;
    //     } 
    // }
    
    // /make register_pipe_name
    makefifo(register_pipe_name);

    // open register pipe
    fserv = open_pipe(register_pipe_name, O_RDONLY);
    int fd = open_pipe(register_pipe_name, O_WRONLY);

    int ret_q;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    // keep reading op_codes from clients
    while(true) {
        ssize_t ret = safe_read(fserv, buffer, BUFFER_SIZE);
        if (ret > 0) ret_q = pcq_enqueue(queue, buffer);
        
        if (ret_q >= 0) {
            memcpy(message, queue->pcq_buffer[ret_q], BUFFER_SIZE);
            
            pcq_dequeue(queue);
        }
        memset(buffer, 0, 1);
    }

    pcq_destroy(queue);
    free(threads_availability);
    destroy_system_boxes();
    close(fd);
    close(fserv);
    return 0;
}
