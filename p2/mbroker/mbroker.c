#include "../utils/logging.h"
#include "../fs/operations.h"
#include "../utils/fifo.h"
#include "boxes.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1200

int fcli, fserv;

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

void send_response(uint8_t op_code, int32_t ret_code) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    switch(op_code) {
        case 4: // create box response
        case 6: // remove box response
            memset(buffer, 0, BUFFER_SIZE);
            buffer[0] = (char) op_code;
            int32_to_bytes(ret_code, buffer, 1);
            memcpy(buffer+5, err_msg, strlen(err_msg));
            ssize_t ret = write(fcli, buffer, BUFFER_SIZE);
            if (ret < 0) exit(EXIT_FAILURE);
            break;
        case 8: // list boxes
            if (num_of_boxes == 0) {
                buffer[1] = 1;
                ret = write(fcli, buffer, BUFFER_SIZE);
                break;
            }
            for (int i = 0; i < num_of_boxes; i++) {
                memset(buffer, 0, BUFFER_SIZE);
                buffer[0] = (char) op_code;
                buffer[1] = (char) system_boxes[i]->last;
                memcpy(buffer+2, system_boxes[i]->box_name, 32);
                uint64_to_bytes(system_boxes[i]->box_size, buffer, 34); 
                uint64_to_bytes(system_boxes[i]->n_publishers, buffer, 42); 
                uint64_to_bytes(system_boxes[i]->n_subscribers, buffer, 50); 
                ret = write(fcli, buffer, BUFFER_SIZE);
                if (ret < 0) exit(EXIT_FAILURE);
            }
            break;

        default: break;
    }
}

void read_pipe_name(char pipe_name[]) {
    ssize_t ret = read(fserv, pipe_name, 256);
    if (ret < 0) exit(EXIT_FAILURE);
}
void read_box_name(char box_name[]) {
    ssize_t ret = read(fserv, box_name, 32);
    if (ret < 0) exit(EXIT_FAILURE);
}
void read_pipe_and_box_name(char pipe_name[], char box_name[]) {
    read_pipe_name(pipe_name);
    read_box_name(box_name);
}

void request_handler(char *op_code) {
    ssize_t ret;
    char buffer[BUFFER_SIZE];
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
            system_boxes[index]->n_publishers = 1;
            int fd = tfs_open(box_name, TFS_O_APPEND);
            if (fd == -1) exit(EXIT_FAILURE);
            system_boxes[index]->fd = fd;

            while(true) {
                ret = read(fcli, buffer, BUFFER_SIZE);
                if (ret <= 0) break;

                puts(buffer);

                size_t len = strlen(buffer);
                printf("A len eh %zu\n", len);
                system_boxes[index]->box_size += len;

                buffer[len -1] = '\0';

                ret = tfs_write(fd, buffer, len);
                if (ret == -1) exit(EXIT_FAILURE);
            }
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

            // tem que ficar a ler a box
            close(fcli);
            break;
    
        case 3: // create box
            read_pipe_and_box_name(pipe_name, box_name);
            fcli = open_pipe(pipe_name, O_WRONLY);
            send_response(4, create_box(box_name));
            close(fcli);
            break;
        
        case 5: // remove box
            read_pipe_and_box_name(pipe_name, box_name);
            fcli = open_pipe(pipe_name, O_WRONLY);
            send_response(6, remove_box(box_name));
            close(fcli);
            break;
        
        case 7: // list     
            read_pipe_name(pipe_name);
            fcli = open_pipe(pipe_name, O_WRONLY);
            send_response(8, 0);
            puts("SUCCESS: Boxes listed");
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
    if (tfs_init(NULL) == -1) {
        return -1;
    }

    char *register_pipe_name = argv[1];

    // /make register_pipe_name
    makefifo(register_pipe_name);

    // open register pipe
    fserv = open_pipe(register_pipe_name, O_RDONLY);

    char op_code[1];
    // keep reading op_codes from clients
    while(true) {
        ssize_t ret = read(fserv, op_code, 1);
        if (ret < 0) break;
        else if (ret > 0) request_handler(op_code);
    }
    close(fserv);
    close(fcli);
    return 0;
}
