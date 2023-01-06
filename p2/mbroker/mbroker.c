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

int get_request(char *request) {
    switch((uint8_t)request[0]) {
        case (uint8_t)1:
            char pipe_name, box_name;
            sscanf(request+1, "%s %s", &pipe_name, &box_name);
            //int v = tfs_open(&box_name, TFS_O_APPEND);
            int v = 0;
            int fcli = open(&pipe_name, O_WRONLY);
            ssize_t ret;
            if (v == -1) {
                ret = write(fcli, "-1", 3);
                if (ret < 0) return -1;
                close(fcli);
                open(&pipe_name, O_RDONLY);
            }
            else { 
                ret = write(fcli, "0", 2);
                if (ret < 0) return -1;
                close(fcli);
            }
            break;
        default:
            return -1;
    }
    return 0;
}


int main(int argc, char **argv) {
    (void) argv;
    if (argc != 3) {
        fprintf(stderr, "usage: mbroker <register_pipe_name> <max_sessions>\n");
        return -1;
    }

    if (tfs_init(NULL) == -1) {
        return -1;
    }

    char *register_pipe_name = argv[1];
    unlink(register_pipe_name);
    makefifo(register_pipe_name);

    int fserv = open(register_pipe_name, O_RDONLY);
    if (fserv < 0) return -1;
    char request[BUFFER_SIZE];
    ssize_t req;
    req = read(fserv, request, BUFFER_SIZE-1);
    if (req < 0) return -1; 
    get_request(request);

/*
    ssize_t ret;
    char buffer[BUFFER_SIZE];
   
    while(true) {
        ret = read(fserv, buffer, BUFFER_SIZE - 1);
        if (ret < 0) {
            exit(-1);
        }
        buffer[ret] = 0;
        fputs(buffer, stdout);        
    }
    close(fserv);
    return 0;
*/
}
