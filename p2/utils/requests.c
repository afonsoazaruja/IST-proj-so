#include "../utils/logging.h"
#include "../utils/fifo.h"
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int send_request(uint8_t code, char *register_pipe_name, char *pipe_name, char *box_name)  {
    // build request
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE-1);
    buffer[0] =(char) code;
    memcpy(buffer+1, pipe_name, strlen(pipe_name));
    memcpy(buffer+1+256, box_name, strlen(box_name));

    // open pipe to send request to mbroker
    int fserv = open(register_pipe_name, O_WRONLY);
    if (fserv < 0) return -1;
    
    // send request to mbroker
    ssize_t ret = write(fserv, buffer, sizeof(buffer));
    if (ret < 0) return -1;

    close(fserv);
    return 0;
}

int send_request_to_list_boxes(int code, char *register_pipe_name, char *pipe_name) {
    // build request
    char buffer[strlen(pipe_name) + 1];
    memset(buffer, 0, sizeof(buffer));
    buffer[0] =(char) code;
    memcpy(buffer+1, pipe_name, strlen(pipe_name));

    // open pipe to send request to mbroker
    int fserv = open(register_pipe_name, O_WRONLY);
    if (fserv < 0) return -1;
    
    // send request to mbroker
    ssize_t ret = write(fserv, buffer, sizeof(buffer));
    if (ret < 0) return -1;
    close(fserv);
    return 0;
}