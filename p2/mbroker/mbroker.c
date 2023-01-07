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

#define BUFFER_SIZE 1024

int fcli, fserv;

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

void request_handler(char *op_code) {
    char buffer[BUFFER_SIZE];
    char pipe_name[256];
    char box_name[32];

    switch((uint8_t)op_code[0]) {
        case 1:
            read_data(buffer, pipe_name, box_name);
            puts("SUCCESS: Publisher connected");
            /*
             if (...) {
                int fcli = open(pipe_name, 0_WRONLY);
                ret = write(fcli, "Connection refused", 18);
                if (ret < 0) exit(EXIT_FAILURE);
                close(fcli);
            }
            */
            break;
        case 2:
            read_data(buffer, pipe_name, box_name);
            puts("SUCCESS: Subscriber connected");
            break;
        case 3:
            read_data(buffer, pipe_name, box_name);
            puts("SUCCESS: Box created");
            break;
        case 4:

        case 5:
            read_data(buffer, pipe_name, box_name);
            puts("SUCCESS: Box removed");
            break;
        case 7:
            read_data(buffer, pipe_name, box_name);
            puts("SUCCESS: Boxes listed");
            break;
            
        default: return;
    }
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
    close(fcli);
    close(fserv);
    return 0;
}
