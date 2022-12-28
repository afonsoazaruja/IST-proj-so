#include "../fs/operations.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t const file_contents[] = "AAA!";
char const target_path1[] = "/f1";
char const link_path1[] = "/l1";
char const link_path2[] = "/l2";

void assert_empty_file(char const *path) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(file_contents)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);
}

int main() {
    assert(tfs_init(NULL) != -1);

    // create file target_path1
    assert(tfs_open(target_path1, TFS_O_CREAT) != -1);
    
    assert_empty_file(target_path1);

    // create soft link link_path1 to target_path1
    assert(tfs_sym_link(target_path1, link_path1) != -1);

    assert_empty_file(link_path1);

    // create hard link link_path2 to link_path1
    assert(tfs_link(link_path1, link_path2) == -1);

    assert_empty_file(link_path2);

    assert(tfs_destroy() != -1);

    printf("Successful test.\n");

    return 0;
}
