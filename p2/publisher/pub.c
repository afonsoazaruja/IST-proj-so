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
#include <signal.h>

#define BUFFER_SIZE 1024

int fcli;

void sigpipe_handler() {
    close(fcli);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    (void)argv;
    if (argc != 4) {
        fprintf(stderr, "usage: pub <register_pipe_name> <pipe_name> <box_name>\n");
        return -1;
    }
    
    char *register_pipe_name = argv[1];
    char *pipe_name = argv[2];
    char *box_name = argv[3];

    // sigaction struct to detect SIGPIPE    
    struct sigaction sa;
    sa.sa_handler = sigpipe_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, NULL);
    
    // make pipe_name
    makefifo(pipe_name);

    if (send_request(1, register_pipe_name, pipe_name, box_name) == -1) return -1;

    // open pipe to receive response from mbroker
    fcli = open_pipe(pipe_name, O_WRONLY);

    // check if pipe is still open
    ssize_t ret = safe_write(fcli, "", 1);
    if (ret < 0) return -1;

    char buffer[BUFFER_SIZE];
    buffer[0] = (char) 9;
    // publisher sends messages
    while(fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        ret = safe_write(fcli, buffer, BUFFER_SIZE);
        memset(buffer+1, 0, BUFFER_SIZE-1);
    }
    close(fcli);
    return 0;
}
