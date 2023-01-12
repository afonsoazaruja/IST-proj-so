#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void makefifo(const char *path) {
    if (unlink(path) == -1) {
        fprintf(stderr, "[ERR]: unlink failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (mkfifo(path, 0777) == -1) {
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int open_pipe(const char *path, int flag) {
    int pipe = open(path, flag);
    if (pipe < 0) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return pipe;
}