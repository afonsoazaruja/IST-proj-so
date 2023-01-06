#include "utils/logging.h"
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

int main(int argc, char **argv) {
    (void)argv;

    if (argc != 4) {
        fprintf(stderr, "usage: pub <register_pipe_name> <pipe_name> <box_name>\n");
        return -1;
    }

    char *register_pipe_name = argv[1];
    char *pipe_name = argv[2];
    char *box_name = argv[3];

    // make pipe_name
    unlink(register_pipe_name);
    makefifo(pipe_name);

    // open pipe to send request to mbroker
    int fserv = open(register_pipe_name, O_WRONLY);
    if (fserv < 0) return -1;
    
    // open pipe to receive response from mbroker
    int fcli = open(pipe_name, O_RDONLY);
    if (fcli < 0) return -1;

    // build request
    char buffer[BUFFER_SIZE];
    buffer[0] = (char)1;
    memset(buffer, 0, BUFFER_SIZE-1);
    memcpy(buffer+1, pipe_name, 256);
    memcpy(buffer+1+256, box_name, 32);

    ssize_t check;
    // send request to mbroker
    check = write(fserv, buffer, BUFFER_SIZE-1);
    if (check < 0) return -1;
    // read response from mbroker
    memset(buffer, 0, BUFFER_SIZE-1);
    check = read(fcli, buffer, BUFFER_SIZE-1);
    // do something with that...
    close(fcli);
    if (strcmp(buffer, "-1") == 0) {
        return -1;
    }
    fcli = open(pipe_name, O_WRONLY);
    if (fcli < 0) return -1;
    printf("aqui\n");

/*
    ssize_t ret;
    while(fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        ret = write(tx, buffer, BUFFER_SIZE - 1);

        if (ret == -1) {
            fprintf(stderr, "Error.");
            return -1;
        }

        ack_ret = read(ack_rx, &ack, 1);
        if (ack_ret < 0) {
           fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    close(ack_rx);
    close(tx);
*/
    return 0;
}
