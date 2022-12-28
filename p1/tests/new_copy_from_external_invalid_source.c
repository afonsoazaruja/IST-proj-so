#include "../fs/operations.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {

    char *path_invalid = "invalid";
    char *path_src = "tests/empty_file.txt";

    assert(tfs_init(NULL) != -1);

    int f;

    f = tfs_copy_from_external_fs(path_src, path_invalid);
    assert(f == -1);

    printf("Successful test.\n");

    return 0;
}
