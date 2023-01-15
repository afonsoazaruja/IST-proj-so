#ifndef BOXES_H
#define BOXES_H

#include "../utils/logging.h"
#include "../fs/operations.h"
#include "../utils/pipes.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define ERR_SIZE 1024
#define MAX_BOXES 64
#define BOX_NAME_SIZE 32

typedef enum { FREE = 0, TAKEN = 1 } allocation_state_t;

typedef struct {
    allocation_state_t state;
    char box_name[BOX_NAME_SIZE];
    uint64_t box_size;
    uint64_t n_publishers;
    uint64_t n_subscribers;
} box;

extern char err_msg[ERR_SIZE];
extern box **system_boxes;
extern int num_of_boxes;

void destroy_system_boxes();
void init_boxes();
int find_box(char *box_name);
int remove_box(char *box_name);
int create_box(char *box_name);

#endif