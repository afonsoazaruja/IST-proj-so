#include "fs/operations.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    const char *existing_path = "/a1";
    const char *file_path = "/f1";

    assert(tfs_init(NULL) != -1);

    // file_path doesn't exist still
    assert(tfs_sym_link(file_path, existing_path) == -1);
    assert(tfs_link(file_path, existing_path) == -1);

    // create file file_path
    int fd = tfs_open(file_path, TFS_O_CREAT);
    assert(fd != -1);

    // create file existing_path
    int fd_exisiting = tfs_open(existing_path, TFS_O_CREAT);
    assert(fd_exisiting != -1);

    // link must not be created because path already exists
    assert(tfs_sym_link(file_path, existing_path) == -1);
    assert(tfs_link(file_path, existing_path) == -1);

    // close file
    assert(tfs_close(fd) != -1);

    printf("Successful test.\n");
}
