#include "../utils/logging.h"
#include "../fs/operations.h"
#include "../utils/fifo.h"
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

int fcli, fserv;
char err_msg[ERR_SIZE];

void send_response(int op_code, int r_code, char *buffer) {
    switch(op_code) {
        case 4: // create box response
        case 6: // remove box response
            memset(buffer, 0, BUFFER_SIZE-1);
            buffer[0] = (char)op_code;
            buffer[1] = (char)r_code;    
            memcpy(buffer+1+1, err_msg, strlen(err_msg));
            ssize_t ret = write(fcli, buffer, BUFFER_SIZE-1);
            if (ret < 0) exit(EXIT_FAILURE);
            break;
        case 8: // list boxes
            break;
        default: break;
    }
}

void read_data(char *buffer, char *pipe_name, char *box_name) {
    ssize_t ret;
    // read pipe name
    ret = read(fserv, buffer, 256);
    if (ret < 0) exit(EXIT_FAILURE);
    memcpy(pipe_name, buffer, 256);

    // read box name
    ret = read(fserv, buffer + 256, 32);
    if (ret < 0) exit(EXIT_FAILURE);
    memcpy(box_name, buffer + 256, 32);
}

int create_box(char *box_name) {
    int value = is_box_registered(box_name);
    memset(err_msg, 0, ERR_SIZE-1);
    if (value < 0) {
        memcpy(err_msg, "ERROR: Failed search", 21);
        return -1;
    }
    if (value == 1) {
        memcpy(err_msg, "ERROR: Box already exists", 26);
        return -1;
    }
    int fhandle = tfs_open(box_name, TFS_O_CREAT);
    if (fhandle == -1) return -1;
    return 0;
}

int remove_box(char *box_name) {
    int value = is_box_registered(box_name);
    memset(err_msg, 0, ERR_SIZE-1);
    if (value < 0) {
        memcpy(err_msg, "ERROR: Failed search", 21);
        return -1;
    }
    if (value == 0) {
        memcpy(err_msg, "ERROR: Box doesn't exist", 25);
        return -1;
    }
    if (tfs_unlink(box_name) == -1) {
        memcpy(err_msg, "ERROR: Couldn't remove box", 27);
        return -1;
    }
    return 0;
}

void request_handler(char *op_code) {
    ssize_t ret;
    char buffer[BUFFER_SIZE];
    char pipe_name[256];
    char box_name[32];

    switch((uint8_t)op_code[0]) {
        case 1: // create publisher
            read_data(buffer, pipe_name, box_name);
            puts("SUCCESS: Publisher connected");
            fcli = open(pipe_name, O_WRONLY);
            ret = write(fcli, "0", 2);
            if (ret < 0) exit(EXIT_FAILURE);
            close(fcli);
            break;

        case 2: // create subscriber
            read_data(buffer, pipe_name, box_name);
            puts("SUCCESS: Subscriber connected");
            break;
        
        case 3: // create box
            read_data(buffer, pipe_name, box_name);
            fcli = open(pipe_name, O_WRONLY);
            send_response(4, create_box(box_name), buffer);
            close(fcli);
            break;
        
        case 5: // remove box
            read_data(buffer, pipe_name, box_name);
            fcli = open(pipe_name, O_WRONLY);
            send_response(6, remove_box(box_name), buffer);
            close(fcli);
            break;
        
        case 7: // list boxes
            read_data(buffer, pipe_name, box_name);
            fcli = open(pipe_name, O_WRONLY);
            ret = write(fcli, "0", 2);
            if (ret < 0) exit(EXIT_FAILURE);
            close(fcli);
            puts("SUCCESS: Boxes listed");
            break;
            
        default: return;
    }
    return;
}


int main(int argc, char **argv) {
    ssize_t ret;
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
        ret = read(fserv, op_code, 1);
        if (ret <= 0) break;
        request_handler(op_code);
    }
    close(fserv);
    close(fcli);
    return 0;
}
