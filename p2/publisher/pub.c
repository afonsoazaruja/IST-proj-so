#include "../utils/logging.h"
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
    (void)argv;

    if (argc != 4) {
        fprintf(stderr, "usage: pub <register_pipe_name> <pipe_name> <box_name>\n");
        return -1;
    }

    //makefifo("pub1");
    char buffer[BUFFER_SIZE];

    int tx = open(PIPE, O_WRONLY);
    if (tx < 0) return -1;
    
    int ack_rx = open(PIPE_ACK, O_RDONLY);
    if (ack_rx < 0) return -1;

    char ack;

    ssize_t ret;
    ssize_t ack_ret;
    while(!feof(stdin)) {
        if (scanf("%s", buffer) != 1) {
            fprintf(stderr, "Failed to read input.");
        }
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
    return 0;
}
