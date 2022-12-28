#include "../fs/operations.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

void *delete_file(void *arg) {
    char const *path = *((char const**)arg);
    // test if unlink doesn't execute while creating file
    // would return -1 if file doesn't exist
    assert(tfs_unlink(path) != -1);
    return NULL;
}


void *create_file(void *arg) {
    char const *path = *((char const**)arg);
    int fhandle = tfs_open(path, TFS_O_CREAT | TFS_O_APPEND);
    tfs_close(fhandle);
    return NULL;
}

int main() {
    assert(tfs_init(NULL) != -1);
    char const *path = "/test_path";

    pthread_t tid[2];

   if (pthread_create(&tid[0], NULL, create_file, (void*)&path) != 0) {
        fprintf(stderr, "failed to create create_file thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&tid[1], NULL, delete_file, (void*)&path) != 0) {
        fprintf(stderr, "failed to create delete_file thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    // assert that file doesn't exist
    assert(tfs_open(path, TFS_O_APPEND) == -1); 

    printf("Successful test.\n");

    return 0;
}
