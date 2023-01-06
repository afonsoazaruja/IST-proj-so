#include "utils/logging.h"
#include "fs/operations.h"
#include "utils/fifo.h"
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

void request_handler(char *op_code) {
    char buffer[BUFFER_SIZE];
    char pipe_name[256];
    char box_name[32];
    switch((uint8_t)op_code[0]) {
        case (uint8_t)1:
        case (uint8_t)2:
            ssize_t ret;
            // read pipe_name
            ret = read(fserv, buffer, 256);
            if (ret < 0) exit(EXIT_FAILURE);
            memcpy(pipe_name, buffer, 256);
            // read box_name
            ret = read(fserv, buffer+256, 32);
            if (ret < 0) exit(EXIT_FAILURE);
            memcpy(box_name, buffer+256, 32);

            puts("SUCCESS: Subscriber connected");
        case (uint8_t)3:
        default:
            return;
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
