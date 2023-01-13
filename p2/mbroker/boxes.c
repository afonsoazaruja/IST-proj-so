#include "../utils/logging.h"
#include "../fs/operations.h"
#include "../utils/fifo.h"
#include "boxes.h"
#include <errno.h>
#include <fcntl.h>

char err_msg[ERR_SIZE];
box **system_boxes;
long num_of_boxes = 0;

void destroy_system_boxes() {
    for (int i = 0; i < num_of_boxes; i++) {
        free(system_boxes[i]);
        system_boxes[i] = NULL;
    }
    free(system_boxes);
    system_boxes = NULL;
}

long find_box(char *box_name) {
    long index = -1;

    for (long i = 0; i < num_of_boxes; i++) {
        if (strcmp(system_boxes[i]->box_name, box_name) == 0) {
            index = i;
            break;
        }
    }
    return index;
}

int32_t remove_box(char *box_name) {
    int value = is_box_registered(box_name);
    memset(err_msg, 0, ERR_SIZE-1);
    if (value < 0) {
        memcpy(err_msg, "ERROR: Failed search", 21);
        return -1;
    }
    if (value == 0) {
        memcpy(err_msg, "NO BOXES FOUND", 14);
        return -1;
    }
    if (tfs_unlink(box_name) == -1) {
        memcpy(err_msg, "ERROR: Couldn't remove box", 27);
        return -1;
    }

    long index_of_box = find_box(box_name);
    // Remove the box from the system_boxes array
    for (long i = index_of_box; i < num_of_boxes - 1; i++) {
        system_boxes[i] = system_boxes[i+1];
    }
    num_of_boxes--;

    return 0;
}

int comparator(const void *b1, const void *b2) {
    return strcmp(((box*) b1)->box_name, ((box*) b2)->box_name);
}

void resize_system_boxes(box *new_box) {
    box **new_system_boxes;

    new_system_boxes = malloc(sizeof(box*) * ((unsigned long)num_of_boxes + 1));
    memcpy(new_system_boxes, system_boxes, sizeof(box*) * (unsigned long)num_of_boxes);
    new_system_boxes[num_of_boxes++] = new_box;

    if (num_of_boxes > 1) system_boxes[num_of_boxes-2]->last = 0;

    free(system_boxes);
    system_boxes = new_system_boxes;
}

int32_t create_box(char *box_name) {
    long value = find_box(box_name);
    memset(err_msg, 0, ERR_SIZE-1);
    if (value >= 0) {
        memcpy(err_msg, "ERROR: Box already exists", 26);
        return -1;
    }
    int fhandle = tfs_open(box_name, TFS_O_CREAT);
    if (fhandle == -1) return -1;

    box *new_box = malloc(sizeof(box));
    memcpy(new_box->box_name, box_name, strlen(box_name));
    new_box->n_publishers = 0;
    new_box->n_subscribers = 0;
    new_box->box_size = 0;
    new_box->last = 1;

    // Resize the system_boxes array and add new_box
    resize_system_boxes(new_box);
    list_boxes();

   
    return 0;
}

void list_boxes() {
    for (int i = 0; i < num_of_boxes; i++) {
        fprintf(stdout, "%s %zu %zu %zu\n", system_boxes[i]->box_name,
        system_boxes[i]->box_size, system_boxes[i]->n_publishers, 
        system_boxes[i]->n_subscribers);
    }
}
