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

int fcli;

void request_handler(char *request) {
    char *pipe_name = NULL;
    char *box_name = NULL;
    switch((uint8_t)request[0]) {
        case (uint8_t)1:
            sscanf(request+1, "%s %s", pipe_name, box_name);
            // clear buffer to send response to client
            memset(request, 0, BUFFER_SIZE-1);
            //int v = tfs_open(&box_name, TFS_O_APPEND);
            int v = 0;
            // open pipe_name in O_WRONLY to send response to client
            fcli = open(pipe_name, O_WRONLY);
            ssize_t ret;
            if (v == -1) {
                fprintf(stderr, "ERROR: Could not connect publisher\n");
                memcpy(request, "-1", 3);
                ret = write(fcli, request, BUFFER_SIZE-1);
                if (ret < 0) exit(-1);
                close(fcli);
            }
            else { 
                fprintf(stdout, "SUCCESS: Publisher connected\n");
                memcpy(request, "0", 2);
                ret = write(fcli, request, BUFFER_SIZE-1);
                if (ret < 0) exit(-1);
                close(fcli);
                open(pipe_name, O_RDONLY);
            }
            break;
        case (uint8_t)2:
            sscanf(request+1, "%s %s", pipe_name, box_name);
            memset(request, 0, BUFFER_SIZE-1);
            /*
            int v = 0;
            if (v == -1) break;
            else {
            */
                puts("SUCCESS: Subscriber connected");
                //fprintf(stdout, "SUCCESS: Subscriber connected\n");
        default:
            return;
    }
}

int main(int argc, char **argv) {
    int fserv;
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

    // keep reading requests from clients
    char buffer[BUFFER_SIZE];
    while(true) {
        ret = read(fserv, buffer, BUFFER_SIZE-1);
        if (ret <= 0) break;
        request_handler(buffer);
    }
    close(fcli);
    close(fserv);
    return 0;
}
