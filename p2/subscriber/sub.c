#include "../utils/logging.h"
#include "../utils/fifo.h"
#include "../utils/requests.h"
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

    char *register_pipe_name = argv[1];
    char *pipe_name = argv[2];
    char *box_name = argv[3];

    // make pipe_name
    makefifo(pipe_name);
    
    if (send_request(2, register_pipe_name, pipe_name, box_name) == -1) return -1;

    // open pipe to receive response from mbroker
    int fcli = open_pipe(pipe_name, O_RDONLY);

    // read response from mbroker
    char buffer[3];
    ssize_t ret = read(fcli, buffer, 3);
    if (ret < 0) return -1;

    close(fcli);
    if (strcmp(buffer, "-1") == 0) {
        return -1;
    }
    else {
        fprintf(stdout, "SUCCESS");
    }
    return 0;
}
