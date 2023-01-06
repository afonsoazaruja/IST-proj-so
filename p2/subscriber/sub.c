#include "logging.h"
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
        fprintf(stderr, "usage: sub <register_pipe_name> <pipe_name> <box_name>\n");
        return -1;
    }

    char *register_pipe_name = argv[1];
    char *pipe_name = argv[2];
    char *box_name = argv[3];

    // make pipe_name
    unlink(pipe_name);
    makefifo(pipe_name);

    // build request from publisher
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE-1);
    buffer[0] = (char)2;
    memcpy(buffer+1, pipe_name, strlen(pipe_name));
    memcpy(buffer+257, box_name, strlen(box_name));

    printf("%s\n", buffer);
    puts(buffer);

    // open pipe to send request to mbroker
    int fserv = open(register_pipe_name, O_WRONLY);
    if (fserv < 0) return -1;

    ssize_t ret;
    // send request to mbroker
    ret = write(fserv, buffer, BUFFER_SIZE-1);
    if (ret < 0) return -1;
    
    // open pipe to receive responso from mbroker
    int fcli = open(pipe_name, O_RDONLY);
    if (fcli < 0) return -1;
    
    // read response from mbroker
    ret = read(fcli, buffer, BUFFER_SIZE-1);
    if (ret < 0) return -1;

    close(fcli);
    if (strncmp(buffer, "-1", 1) == 0) {
        close(fcli);
        close(fserv);
        return -1;
    }
    else {
        fprintf(stdout, "SUCCESS");
    }

    return 0;
}
