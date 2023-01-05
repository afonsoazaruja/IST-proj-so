#include "logging.h"

int main(int argc, char **argv) {
    (void)argv;
    if (argc != 4) {
        fprintf(stderr, "usage: sub <register_pipe_name> <box_name>\n");
        return -1;
    }
    return 0;
}
