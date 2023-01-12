#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void makefifo(const char *path) {
    if (unlink(path) == -1) {
        fprintf(stderr, "[ERR]: unlink failed: %s\n", strerror(errno));
    }
    if (mkfifo(path, 0777) == -1) {
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}