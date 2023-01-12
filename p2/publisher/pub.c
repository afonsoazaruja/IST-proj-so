#include "../utils/logging.h"
#include "../utils/fifo.h"
#include "../utils/requests.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

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
    makefifo(pipe_name);

   if (send_request(1, register_pipe_name, pipe_name, box_name) == -1) return -1;

    // open pipe to receive response from mbroker
    int fcli = open(pipe_name, O_RDONLY);
    if (fcli < 0) return -1;

    // read response from mbroker
    char code[3];
    ssize_t ret = read(fcli, code, 3);
    if (ret < 0) return -1;
    close(fcli);
    // if publisher couldn't be created
    if (strcmp(code, "-1") == 0) {
        puts("ola\n");
        return -1;
    }
    else {
        fcli = open(pipe_name, O_WRONLY);
    }
    char buffer[BUFFER_SIZE];
    // publisher sends messages
    while(fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        ret = write(fcli, buffer, BUFFER_SIZE);
        if (ret < 0) exit(EXIT_FAILURE);
    }

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
