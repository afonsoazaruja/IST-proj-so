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
        fprintf(stderr, "usage: sub <register_pipe_name> <pipe_name> <box_name>\n");
        return -1;
    }

    if (send_request(2, argv[1], argv[2], argv[3]) == -1) return -1;

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

    close(fcli);
    if (strncmp(buffer, "-1", 1) == 0) {
        return -1;
    }
    else {
        fprintf(stdout, "SUCCESS");
    }
    return 0;
}
