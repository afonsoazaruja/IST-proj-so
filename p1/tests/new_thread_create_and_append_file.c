#include "fs/operations.h"
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


void *create_file(void *arg) {
    char const *path = *((char const**)arg);
    int fhandle = tfs_open(path, TFS_O_CREAT | TFS_O_APPEND);
    tfs_write(fhandle, "test ", 5);
    tfs_close(fhandle);
    return NULL;
}

int main() {
    assert(tfs_init(NULL) != -1);
    char const *path = "/test_path";
    uint8_t const file_contents[] = "test test ";

    pthread_t tid[2];

   if (pthread_create(&tid[0], NULL, create_file, (void*)&path) != 0) {
        fprintf(stderr, "failed to create create_file thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    sleep(1);
    if (pthread_create(&tid[1], NULL, create_file, (void*)&path) != 0) {
        fprintf(stderr, "failed to create create_file thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    int f = tfs_open(path, TFS_O_CREAT);
    assert(f != -1);

    uint8_t buffer[sizeof(file_contents)];

    assert(tfs_read(f, buffer, sizeof(buffer)) != -1);
    assert(memcmp(buffer, file_contents, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);

    printf("Successful test.\n");

    return 0;
}
