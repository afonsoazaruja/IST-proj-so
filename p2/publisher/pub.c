#include "../utils/logging.h"
#include "../utils/fifo.h"
#include "../utils/request_control.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char **argv) {
    (void)argv;
    if (argc != 4) {
        fprintf(stderr, "usage: pub <register_pipe_name> <pipe_name> <box_name>\n");
        return -1;
    }

   if (send_request(1, argv[1], argv[2], argv[3]) == -1) return -1;

    // make pipe_name
    char *pipe_name = argv[2];
    unlink(pipe_name);
    makefifo(pipe_name);

    // open pipe to receive response from mbroker
    int fcli = open(pipe_name, O_RDONLY);
    if (fcli < 0) return -1;

    // read response from mbroker
    char buffer[BUFFER_SIZE];
    ssize_t ret = read(fcli, buffer, BUFFER_SIZE-1);
    if (ret < 0) return -1;

    if (strcmp(buffer, "Connection refused") == 0) {
        // (...)
    } else {
        // (...)
    }
    
    // close pipe to open it in O_WRONLY because it is a publisher
    close(fcli);
    if (strcmp(buffer, "-1") == 0) {
        return -1;
    }
    fcli = open(pipe_name, O_WRONLY);
    fprintf(stdout, "SUCCESS\n");

    //close(fserv);
    //if (fcli < 0) return -1;
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
