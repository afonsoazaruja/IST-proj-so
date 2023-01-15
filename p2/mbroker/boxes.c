#include "../utils/logging.h"
#include "../fs/operations.h"
#include "../utils/pipes.h"
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

void init_boxes() {
    system_boxes = malloc(sizeof(box) * ((size_t) (MAX_BOXES)));
    for (int i = 0; i < MAX_BOXES; i++) {
        system_boxes[i] = malloc(sizeof(box));
        system_boxes[i]->state = FREE;
        system_boxes[i]->box_name[0] = '\0';
    }
}

int find_box(char *box_name) {
    int index = -1;

    for (int i = 0; i < num_of_boxes; i++) {
        if (system_boxes[i]->state == TAKEN &&
        strcmp(system_boxes[i]->box_name, box_name) == 0) {
            index = i;
            break;
        }
    }
    return index;
}

int remove_box(char *box_name) {
    memset(err_msg, 0, ERR_SIZE-1);
    int index = find_box(box_name);
    if (index < 0) {
        memcpy(err_msg, "NO BOXES FOUND", 15);
        return -1;
    }
    if (tfs_unlink(box_name) == -1) {
        memcpy(err_msg, "ERROR Couldn't remove box", 26);
        return -1;
    }

    free(system_boxes[index]);
    system_boxes[index] = malloc(sizeof(box));
    system_boxes[index]->state = FREE;
    system_boxes[index]->box_name[0] = '\0';
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
        if (system_boxes[i]->state == FREE) {
            system_boxes[i]->state = TAKEN;
            memcpy(system_boxes[i]->box_name, box_name, strlen(box_name));
            system_boxes[i]->n_publishers = 0;
            system_boxes[i]->n_subscribers = 0;
            system_boxes[i]->box_size = 0;
            num_of_boxes++;
            break;
        }
    }
    return 0;
}