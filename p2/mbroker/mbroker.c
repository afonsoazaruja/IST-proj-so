#include "../utils/logging.h"
#include "../fs/operations.h"
#include "../utils/fifo.h"
#include "../utils/boxes.h"
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

void send_response(int op_code, int r_code, char *buffer) {
    switch(op_code) {
        case 4: // create box response
        case 6: // remove box response
            memset(buffer, 0, BUFFER_SIZE-1);
            buffer[0] = (char)op_code;
            buffer[1] = (char)r_code;    
            memcpy(buffer+2, err_msg, strlen(err_msg));
            ssize_t ret = write(fcli, buffer, BUFFER_SIZE-1);
            if (ret < 0) exit(EXIT_FAILURE);
            break;
        case 8: // list boxes
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

    switch((uint8_t)op_code[0]) {
        case 1: // create publisher
            read_pipe_and_box_name(pipe_name, box_name);
            puts("SUCCESS: Publisher connected");
            fcli = open(pipe_name, O_WRONLY);
            ret = write(fcli, "0", 2);
            if (ret < 0) exit(EXIT_FAILURE);
            close(fcli);
            break;

        case 2: // create subscriber
            read_pipe_and_box_name(pipe_name, box_name);
            puts("SUCCESS: Subscriber connected");
            break;
        
        case 3: // create box
            read_pipe_and_box_name(pipe_name, box_name);
            fcli = open(pipe_name, O_WRONLY);
            send_response(4, create_box(box_name), buffer);
            close(fcli);
            break;
        
        case 5: // remove box
            read_pipe_and_box_name(pipe_name, box_name);
            puts(pipe_name);
            fcli = open(pipe_name, O_WRONLY);
            // PQ EH QUE TAS A ENVIAR O OP_CODE A 6? PQ NAO 5?        /// ATENCAO ///
            send_response(6, remove_box(box_name), buffer);
            close(fcli);
            break;
        
        case 7: // list boxes
            read_pipe_name(pipe_name);
            list_boxes();
            puts("SUCCESS: Boxes listed");
            fcli = open(pipe_name, O_WRONLY);
            ret = write(fcli, "0", 2);
            if (ret < 0) exit(EXIT_FAILURE);
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
    unlink(register_pipe_name);
    makefifo(register_pipe_name);

    // open register pipe
    fserv = open(register_pipe_name, O_RDONLY);
    if (fserv < 0) return -1;

    // keep reading op_codes from clients
    char op_code[1];
    while(true) {
        ssize_t ret = read(fserv, op_code, 1);
        if (ret <= 0) break;
        request_handler(op_code);
    }
    close(fserv);
    close(fcli);
    return 0;
}
