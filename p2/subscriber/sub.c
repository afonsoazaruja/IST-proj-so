#include "../utils/logging.h"
#include "../utils/pipes.h"
#include "../utils/requests.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

#define BUFFER_SIZE 1024

int fcli, num_of_msgs = 0;

void signal_handler() {
    fprintf(stdout, "%d", num_of_msgs);
    close(fcli);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    (void)argv;
    if (argc != 4) {
        fprintf(stderr, "usage: sub <register_pipe_name> <pipe_name> <box_name>\n");
        return -1;
    }

    char *register_pipe_name = argv[1];
    char *pipe_name = argv[2];
    char *box_name = argv[3];

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    // make pipe_name
    makefifo(pipe_name);
    
    if (send_request(2, register_pipe_name, pipe_name, box_name) == -1) return -1;

    // open pipe to receive response from mbroker
    fcli = open_pipe(pipe_name, O_RDONLY);

    // read response from mbroker
    char buffer[BUFFER_SIZE];
    ssize_t ret;

    while(true) {
        memset(buffer, 0, BUFFER_SIZE);
        ret = safe_read(fcli, buffer, BUFFER_SIZE);
        if (ret == 0) break;
        num_of_msgs++;
        fprintf(stdout, "%s\n", buffer);
    }

    return 0;
}
