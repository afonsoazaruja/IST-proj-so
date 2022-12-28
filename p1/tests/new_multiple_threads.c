#include "../fs/operations.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define NUM_FILES_TO_CREATE (10)
#define NUM_FILES_TO_DELETE (10)

char const *file_names[] = {"test1", "test2", "test3", "test4", "test5", "test6", "test7", "test8",
    "test9", "test10"};

void *create_file(void *arg);
void *delete_file(void *arg);

int main() {

    assert(tfs_init(NULL) != -1);
    pthread_t creaters[NUM_FILES_TO_CREATE];
    pthread_t deleters[NUM_FILES_TO_DELETE];

    for (int i = 0; i < NUM_FILES_TO_CREATE; i++) {
        pthread_create(&creaters[i], NULL, create_file, (void*)&file_names[i]);
    }

    for (int i = 0; i < NUM_FILES_TO_DELETE; i++) {
        pthread_create(&deleters[i], NULL, delete_file, (void*)&file_names[i]);
    }

    for (int i = 0; i < NUM_FILES_TO_DELETE; i++) {
        pthread_join(creaters[i], NULL);
    }

    for (int i = 0; i < NUM_FILES_TO_DELETE; i++) {
        pthread_join(deleters[i], NULL);
    }

    for (int i = 0; i < NUM_FILES_TO_DELETE; i++) {
        // assert that file doesn't exist
        assert(tfs_open(file_names[i], TFS_O_APPEND) == -1);
    }

    printf("Successful test.\n");

    return 0;
}

void *create_file(void *arg) {
    char const *path = *((char const**)arg);
    int fhandle = tfs_open(path, TFS_O_CREAT);
    tfs_close(fhandle);
    return NULL;
}

void *delete_file(void *arg) {
    char const *path = *((char const**)arg);
    assert(tfs_unlink(path) == -1); 
    return NULL;
}


