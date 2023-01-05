#include "logging.h"
#include "fs/operations.h"
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
    (void) argv;
    if (argc != 3) {
        fprintf(stderr, "usage: mbroker <register_pipe_name> <max_sessions>\n");
        return -1;
    }

    if (tfs_init(NULL) == -1) {
        return -1;
    }

    char *register_pipe_name = "fifo";
    makefifo(register_pipe_name);
    int rx = open("fifo", O_RDONLY);
    ssize_t ret;
    while(true) {
        char buffer[BUFFER_SIZE];
        ret = read(rx, buffer, BUFFER_SIZE - 1);
        if (ret == 0) {
            return 0;
        }
        printf("%s", buffer);        
    }

    return 0;

}
