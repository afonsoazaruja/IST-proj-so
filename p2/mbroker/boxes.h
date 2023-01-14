#ifndef BOXES_H
#define BOXES_H

#include "../utils/logging.h"
#include "../fs/operations.h"
#include "../utils/fifo.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define ERR_SIZE 1024

typedef struct {
    char box_name[32];
    uint64_t box_size;
    uint8_t last;
    uint64_t n_publishers;
    uint64_t n_subscribers;
    int fd;
} box;

extern char err_msg[ERR_SIZE];
extern box **system_boxes;
extern int num_of_boxes;

void destroy_system_boxes();
int find_box(char *box_name);
int remove_box(char *box_name);
int comparator(const void *b1, const void *b2);
void resize_system_boxes(box *new_box);
int create_box(char *box_name);
void list_boxes();

#endif