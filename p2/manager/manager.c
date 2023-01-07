#include "../utils/logging.h"
#include <string.h>
#include "../utils/request_control.h"

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> <pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> <pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> <pipe_name> list\n");
}

int main(int argc, char **argv) {
    if (argc != 4 && argc != 5) {
        print_usage();
        return -1;
    }
    if (argc == 4 && strcmp(argv[3], "list") != 0) {
        print_usage();
        return -1;
    }
    if (argc == 5 && strcmp(argv[3], "create") != 0 && strcmp(argv[3], "remove")!= 0) {
        print_usage();
        return -1;
    }
    /*
    char mode = argv[3];
    switch(mode) {
        case :
            break;
        case "remove":
            break;
        case list:
        default:
            break;
    }
    */
    if (strcmp(argv[3], "create") == 0) {
        if (send_request(3, argv[1], argv[2], argv[4]) == -1) return -1;
    } else if (strcmp(argv[3], "remove") == 0) {
        if (send_request(5, argv[1], argv[2], argv[4]) == -1) return -1;
    } else {
        if (send_request_to_list_boxes(7, argv[1], argv[2]) == -1) return -1;
    }

    return 0;
}
