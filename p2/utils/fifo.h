#ifndef FIFO_H
#define FIFO_H

void makefifo(const char *path);
int open_pipe(const char *path, int flag);

#endif