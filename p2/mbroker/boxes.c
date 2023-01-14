#include "../utils/logging.h"
#include "../fs/operations.h"
#include "../utils/fifo.h"
#include "boxes.h"
#include <errno.h>
#include <fcntl.h>

char err_msg[ERR_SIZE];
box **system_boxes;
int num_of_boxes = 0;

void destroy_system_boxes() {
    for (int i = 0; i < num_of_boxes; i++) {
        free(system_boxes[i]);
        system_boxes[i] = NULL;
    }
    free(system_boxes);
    system_boxes = NULL;
}

int find_box(char *box_name) {
    int index = -1;

    for (int i = 0; i < num_of_boxes; i++) {
        if (strcmp(system_boxes[i]->box_name, box_name) == 0) {
            index = i;
            break;
        }
    }
    return index;
}

int remove_box(char *box_name) {
    int value = find_box(box_name);
    memset(err_msg, 0, ERR_SIZE-1);
    if (value < 0) {
        memcpy(err_msg, "NO BOXES FOUND", 15);
        return -1;
    }
    if (tfs_unlink(box_name) == -1) {
        memcpy(err_msg, "ERROR Couldn't remove box", 26);
        return -1;
    }

    int index_of_box = find_box(box_name);
    box *temp = system_boxes[index_of_box];
    // Remove the box from the system_boxes array
    for (int i = index_of_box; i < num_of_boxes - 1; i++) {
        system_boxes[i] = system_boxes[i+1];
    }
    free(temp);
    num_of_boxes--;

    return 0;
}

void resize_system_boxes(box *new_box) {
    system_boxes = realloc(system_boxes, sizeof(box) * ((size_t) (num_of_boxes + 1)));
    system_boxes[num_of_boxes++] = new_box;
}

int create_box(char *box_name) {
    int value = find_box(box_name);
    memset(err_msg, 0, ERR_SIZE-1); 
    if (value >= 0) {
        memcpy(err_msg, "ERROR Box already exists", 25);
        return -1;
    }
    
    int fhandle = tfs_open(box_name, TFS_O_CREAT);
    if (fhandle == -1)  return -1;
    close(fhandle);
    
    box *new_box = malloc(sizeof(box));
    memcpy(new_box->box_name, box_name, strlen(box_name));
    new_box->n_publishers = 0;
    new_box->n_subscribers = 0;
    new_box->box_size = 0;
    new_box->last = 1;

    // Resize the system_boxes array and add new_box
    resize_system_boxes(new_box);

    return 0;
}
