#ifndef FIFO_H
#define FIFO_H

#include <stdio.h>

void makefifo(const char *path);
int open_pipe(const char *path, int flag);int open_pipe(const char *path, int flag);
ssize_t safe_read(int fd, void *buf, size_t _nbytes); 
ssize_t safe_write(int fd, void *buf, size_t _nbytes);

#endif