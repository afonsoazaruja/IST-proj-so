#include "../utils/logging.h"
#include "../fs/operations.h"
#include "../utils/fifo.h"
#include "boxes.h"
#include <errno.h>
#include <fcntl.h>


char err_msg[ERR_SIZE];
box system_boxes[MAX_BOXES];
int num_of_boxes = 0;

void init_boxes() {
    for (int i = 0; i < MAX_BOXES; i++) {
        system_boxes[i].state = FREE;
        system_boxes[i].box_name[0] = '\0';
    }
}

int find_box(char *box_name) {
    int index = -1;

    for (int i = 0; i < MAX_BOXES; i++) {
        if (system_boxes[i].state == TAKEN && strcmp(system_boxes[i].box_name, box_name) == 0) {
            index = i;
            break;
        }
    }
    return index;
}

int remove_box(char *box_name) {
    memset(err_msg, 0, ERR_SIZE-1);
    int i = find_box(box_name);
    if (i < 0) {
        memcpy(err_msg, "Box doesn't exist", 18);
        return -1;
    }
    if (tfs_unlink(box_name) == -1) {
        memcpy(err_msg, "Couldn't remove box", 20);
        return -1;
    }
    // clear box index
    system_boxes[i].state = FREE;
    system_boxes[i].box_name[0] = '\0';
    num_of_boxes--;

    return 0;
}

int create_box(char *box_name) {
    memset(err_msg, 0, ERR_SIZE-1); 
    if (box_name[0] == '\0') {
        memcpy(err_msg, "Invalid box name", 17);
        return -1;
    }
    if (num_of_boxes == MAX_BOXES) {
        memcpy(err_msg, "Max boxes reached", 18);
        return -1;
    }
    if (find_box(box_name) >= 0) {
        memcpy(err_msg, "Box already exists", 19);
        return -1;
    }
    
    int fhandle = tfs_open(box_name, TFS_O_CREAT);
    if (fhandle == -1)  return -1;
    close(fhandle);
    for (int i = 0; i < MAX_BOXES; i++) {
        if (system_boxes[i].state == FREE) {
            system_boxes[i].state = TAKEN;
            memcpy(system_boxes[i].box_name, box_name, BOX_NAME);
            system_boxes[i].n_publishers = 0;
            system_boxes[i].n_subscribers = 0;
            system_boxes[i].box_size = 0;
            break;
        }
    }
    num_of_boxes++;
    return 0;
}
