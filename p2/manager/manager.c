#include "../utils/logging.h"
#include "../utils/requests.h"
#include "../utils/fifo.h"
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1200
#define ERR_SIZE 1024

int fcli; // cliente fifo

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> <pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> <pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> <pipe_name> list\n");
}

// void list_boxes() {
//     for (int i = 0; i < num_of_boxes; i++) {
//         fprintf(stdout, "%s %zu %zu %zu\n", system_boxes[i]->box_name,
//         system_boxes[i]->box_size, system_boxes[i]->n_publishers, 
//         system_boxes[i]->n_subscribers);
//     }
// }

int comparator(const void *b1, const void *b2) {
    char b1_name[32];
    char b2_name[32];

    memcpy(b1_name, (char*) (b1) + 2, 32);
    memcpy(b2_name, (char*) (b2) + 2, 32);
    return strcmp(b1_name, b2_name);
}

uint64_t bytes_to_uint64(char *value) {
    uint64_t num = (uint64_t)value[0] << 56|
                   (uint64_t)value[1] << 48 |
                   (uint64_t)value[2] << 40 |
                   (uint64_t)value[3] << 32 |
                   (uint64_t)value[4] << 24 |
                   (uint64_t)value[5] << 16 |
                   (uint64_t)value[6] << 8  | 
                   (uint64_t)value[7];
    return num;
}

int32_t bytes_to_int32(char *value) {
    int32_t num = (uint64_t)value[0] << 24 |
                  (uint64_t)value[1] << 16 |
                  (uint64_t)value[2] << 8  |
                  (uint64_t)value[3];
    return num;
}


int response_handler(char *op_code) {
    ssize_t ret;
    char ret_code[4];
    char err_msg[ERR_SIZE];
    char buffer[BUFFER_SIZE];
    char boxes[64][BUFFER_SIZE] = {0};
    unsigned long i = 0;

    switch((uint8_t)op_code[0]) { 
        case 4: // response for create box
        case 6: // response for remove box
            ret = safe_read(fcli, ret_code, 4);

            // if box wasn't created or removed
            if (bytes_to_int32(ret_code) < 0) { 
                ret = safe_read(fcli, err_msg, ERR_SIZE);
            }
            else {
                fprintf(stdout, "OK\n");
            }
            break;
        case 8:  
            while (true) {
                memset(buffer, 0, BUFFER_SIZE);
                ret = safe_read(fcli, buffer, BUFFER_SIZE);
                uint8_t last = (uint8_t) buffer[0];
                memcpy(boxes[i], buffer, BUFFER_SIZE);
                i++;
                if (last == 1) break;
            }
            // sort boxes lexicographically
            qsort(boxes, i, BUFFER_SIZE, comparator);

            for (int j = 0; j < i; j++) {
                char box_name[32];
                char box_size[8];
                char n_pub[8];
                char n_sub[8];
                memcpy(box_name, boxes[j]+1, 32);
                memcpy(box_size, boxes[j]+33, 8);
                memcpy(n_pub, boxes[j]+41, 8);
                memcpy(n_sub, boxes[j]+49, 8);

                fprintf(stdout, "%s %zu %zu %zu\n", box_name, 
                    bytes_to_uint64(box_size), 
                    bytes_to_uint64(n_pub), 
                    bytes_to_uint64(n_sub)); 
            }
            break; 

        default:
            break;
    }
    return 0;
}


int main(int argc, char **argv) {
    char *register_pipe_name = argv[1];
    char *pipe_name = argv[2];
    char *mode = argv[3];
    char *box_name = argv[4];

    if ((argc != 4 && argc != 5) || (argc == 4 && strcmp(mode, "list") != 0) ||
        (argc == 5 && strcmp(mode, "create") != 0 && strcmp(mode, "remove")!= 0))
    {
        print_usage();
        return -1;
    }

    // make pipe
    makefifo(pipe_name);
    
    if (strcmp(mode, "create") == 0) {
        if (send_request(3, register_pipe_name, pipe_name, box_name) == -1) return -1;
    } else if (strcmp(mode, "remove") == 0) {
        if (send_request(5, register_pipe_name, pipe_name, box_name) == -1) return -1;
    } else {
        if (send_request_to_list_boxes(7, register_pipe_name, pipe_name) == -1) return -1;
    }

    fcli = open_pipe(pipe_name, O_RDONLY);

    char op_code[1];
    ssize_t ret = read(fcli, op_code, 1);
    if (ret < 0) return -1;

    response_handler(op_code);
    close(fcli);

    return 0;
}
