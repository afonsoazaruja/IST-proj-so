#include "logging.h"

int main(int argc, char **argv) {
    (void) argv;
    if (argc != 3) {
        fprintf(stderr, "usage: mbroker <register_pipe_name> <max_sessions>\n");
        return -1;
    }
    
    return 0;

}
