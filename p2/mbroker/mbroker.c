#include "../utils/logging.h"
#include "../fs/operations.h"
#include "../utils/fifo.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PIPE "/tmp/tst"
#define PIPE_ACK "/tmp/ack"

int main(int argc, char **argv) {
    (void) argv;
    if (argc != 3) {
        fprintf(stderr, "usage: mbroker <register_pipe_name> <max_sessions>\n");
        return -1;
    }
/*
    if (tfs_init(NULL) == -1) {
        return -1;
    }
*/
    
    unlink(PIPE);
    unlink(PIPE_ACK);
    makefifo(PIPE);
    makefifo(PIPE_ACK);

    int rx = open(PIPE, O_RDONLY);
    if (rx < 0) return -1;
    int ack_tx = open(PIPE_ACK, O_WRONLY);
    if (ack_tx < 0) return -1;

    ssize_t ret;
    ssize_t ack_ret;
    char buffer[BUFFER_SIZE];
   
    while(true) {
        ret = read(rx, buffer, BUFFER_SIZE - 1);
        if (ret <= 0) {
            break;
        }
        buffer[ret] = 0;
        fputs(buffer, stdout);        
        
        ack_ret = write(ack_tx, "A", 1);
        
        if (ack_ret < 0) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    close(ack_tx);
    close(rx);
    return 0;
}
