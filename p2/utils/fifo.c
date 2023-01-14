#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

void makefifo(const char *path) {
    if (unlink(path) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (mkfifo(path, 0777) == -1) {
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int open_pipe(const char *path, int flag) {
    int pipe_fd = open(path, flag);
    if (pipe_fd < 0) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return pipe_fd;
}

ssize_t safe_read(int fd, void *buf, size_t _nbytes) {
    ssize_t ret = read(fd, buf, _nbytes);
    if (ret < 0) exit(EXIT_FAILURE);

    return ret;
}

ssize_t safe_write(int fd, void *buf, size_t _nbytes) {
    ssize_t ret = write(fd, buf, _nbytes);
    if (ret < 0) exit(EXIT_FAILURE);

    return ret;
} 