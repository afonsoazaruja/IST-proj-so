#include "../utils/logging.h"
#include "../utils/request_control.h"
#include "../utils/fifo.h"
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1200
#define ERR_SIZE 1024

int fcli; // cliente fifo

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> <pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> <pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> <pipe_name> list\n");
}

int response_handler(char *op_code) {
    ssize_t ret;
    char r_code[1];
    char err_msg[ERR_SIZE];
    switch((uint8_t)op_code[0]) { // TODO CONVERSAO DO RETURN CODE ESTA MAL POR ISSO DA SEMPRE SUCCESS
        case 4:
            // read return code
            ret = read(fcli, r_code, 1);
            if (ret < 0) exit(EXIT_FAILURE);
            if ((uint32_t)r_code[0] < 0) { // if box wasn't created or removed
                ret = read(fcli, err_msg, ERR_SIZE-1);
                if (ret < 0) exit(EXIT_FAILURE);
                puts(err_msg);
            }
            else {
                puts("SUCCESS: Box created");
            }
            break;
        case 6:
            // read return code
            ret = read(fcli, r_code, 1);
            if (ret < 0) exit(EXIT_FAILURE);
            if ((uint32_t)r_code[0] < 0) { // if box wasn't created or removed
                ret = read(fcli, err_msg, ERR_SIZE-1);
                if (ret < 0) exit(EXIT_FAILURE);
                puts(err_msg);
            }
            else {
                puts("SUCCESS: Box removed");
            }
            break;
        case 8:
            break;
        default:
            break;
    }
    return 0;
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

    char *register_pipe_name = argv[1];
    char *pipe_name = argv[2];
    char *mode = argv[3];
    char *box_name = argv[4];

    // make pipe
    unlink(pipe_name);
    makefifo(pipe_name);
    
    if (strcmp(mode, "create") == 0) {
        if (send_request(3, register_pipe_name, pipe_name, box_name) == -1) return -1;
    } else if (strcmp(mode, "remove") == 0) {
        if (send_request(5, register_pipe_name, pipe_name, box_name) == -1) return -1;
    } else {
        if (send_request_to_list_boxes(7, register_pipe_name, pipe_name) == -1) return -1;
    }

    fcli = open(pipe_name, O_RDONLY);
    if (fcli < 0) return -1;

    char op_code[1];
    ssize_t ret = read(fcli, op_code, 1);
    if (ret < 0) return -1;
    response_handler(op_code);
    close(fcli);

    return 0;
}
