#include "../fs/operations.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    const char *existing_path = "/a1";
    const char *file_path = "/f1";

    assert(tfs_init(NULL) != -1);

    // file_path doesn't exist still
    assert(tfs_sym_link(file_path, existing_path) == -1);

    int fd = tfs_open(file_path, TFS_O_CREAT);
    assert(fd != -1);

    int fd_exisiting = tfs_open(existing_path, TFS_O_CREAT);
    assert(fd_exisiting != -1);
    assert(tfs_sym_link(file_path, existing_path) == -1);
    
    // Immediately close file
    assert(tfs_close(fd) != -1);

    printf("Successful test.\n");
}
