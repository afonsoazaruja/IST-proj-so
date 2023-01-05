#include "logging.h"
#include "utils/fifo.h"
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

int main(int argc, char **argv) {
    (void)argv;

    if (argc != 4) {
        fprintf(stderr, "usage: pub <register_pipe_name> <pipe_name> <box_name>\n");
        return -1;
    }

    //makefifo("pub1");
    char buffer[BUFFER_SIZE];

    int tx = open("fifo", O_WRONLY);
    ssize_t ret;
    while(true) {
        if (scanf("%s", buffer) == 0) {
            return 0;
        }
        ret = write(tx, buffer, BUFFER_SIZE - 1);
        if (ret == -1) {
            return 0;
        }
    }
    return 0;
}
