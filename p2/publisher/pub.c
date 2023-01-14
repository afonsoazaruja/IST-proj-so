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
    int fcli = open_pipe(pipe_name, O_WRONLY);

    // read response from mbroker
    char code[3];
    ssize_t ret = write(fcli, code, 3);
    // if publisher couldn't be created
    if (ret <= 0) return -1;

    char buffer[BUFFER_SIZE];
    // publisher sends messages
    while(fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        ret = safe_write(fcli, buffer, BUFFER_SIZE);
    }
    close(fcli);
    return 0;
}
