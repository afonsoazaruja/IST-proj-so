#include "operations.h"
#include "config.h"
#include "state.h"
#include "betterassert.h"
#include "lock_control.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static pthread_mutex_t tfs_mutex;

tfs_params tfs_default_params() {
    tfs_params params = {
        .max_inode_count = 64,
        .max_block_count = 1024,
        .max_open_files_count = 16,
        .block_size = 1024,
    };
    return params;
}

int tfs_init(tfs_params const *params_ptr) {
    tfs_params params;
    if (params_ptr != NULL) {
        params = *params_ptr;
    } else {
        params = tfs_default_params();
    }

    mutex_init(&tfs_mutex);

    if (state_init(params) != 0) {
        return -1;
    }

    // create root inode
    int root = inode_create(T_DIRECTORY);
    if (root != ROOT_DIR_INUM) {
        return -1;
    }

    return 0;
}

int tfs_destroy() {
    if (state_destroy() != 0) {
        return -1;
    }
    mutex_destroy(&tfs_mutex);

    return 0;
}

static bool valid_pathname(char const *name) {
    return name != NULL && strlen(name) > 1 && name[0] == '/';
}

/**
 * Looks for a file.
 *
 * Note: as a simplification, only a plain directory space (root directory only)
 * is supported.
 *
 * Input:
 *   - name: absolute path name
 *   - root_inode: the root directory inode
 * Returns the inumber of the file, -1 if unsuccessful.
 **/

static int tfs_lookup(char const *name, inode_t const *root_inode) {
    ALWAYS_ASSERT(root_inode != NULL, "tfs_lookup: root dir inode must exist");
    if (root_inode != inode_get(ROOT_DIR_INUM)) {
        return -1;
    }
    if (!valid_pathname(name)) {
        return -1;
    }

    // skip the initial '/' character
    name++;

    return find_in_dir(root_inode, name);
}

int tfs_open(char const *name, tfs_file_mode_t mode) {
    // Checks if the path name is valid
    if (!valid_pathname(name)) {
        return -1;
    }

    inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);
    ALWAYS_ASSERT(root_dir_inode != NULL,
                  "tfs_open: root dir inode must exist");
    // so 2 threads do not create the same file - let the file be created first
    mutex_lock(&tfs_mutex);
    int inum = tfs_lookup(name, root_dir_inode);
    size_t offset;


    if (inum >= 0) {
        // The file already exists
        mutex_unlock(&tfs_mutex);
        inode_t *inode = inode_get(inum);
        ALWAYS_ASSERT(inode != NULL,
                      "tfs_open: directory files must have an inode");

        // Truncate (if requested)
        if (mode & TFS_O_TRUNC) {
            if (inode->i_size > 0) {
                data_block_free(inode->i_data_block);
                inode->i_size = 0;
            }
        }
        // Determine initial offset
        offset = (mode & TFS_O_APPEND) ? inode->i_size : 0;

    } else if (mode & TFS_O_CREAT) {
        // The file does not exist; the mode specified that it should be created
        // Create inode
        inum = inode_create(T_FILE);
        if (inum == -1) {
            mutex_unlock(&tfs_mutex);
            printf("no space\n");
            return -1; // no space in inode table
        }

        // Add entry in the root directory
        if (add_dir_entry(root_dir_inode, name + 1, inum) == -1)
        {
            mutex_unlock(&tfs_mutex);
            inode_delete(inum);
            printf("no space dir\n");
            return -1; // no space in directory
        }
        mutex_unlock(&tfs_mutex);
        offset = 0;

    } else {
        mutex_unlock(&tfs_mutex);
        return -1;
    }

    inode_t *inode_file = inode_get(inum);

    if (inode_file->i_node_type == T_SYM_LINK) {
        char *res = data_block_get(inode_file->i_data_block);
        if (tfs_lookup(res, root_dir_inode) == -1) {
            return -1;
        }
        return tfs_open(res, mode);
    }
    // Finally, add entry to the open file table and return the corresponding
    // handle
    return add_to_open_file_table(inum, offset);

    // Note: for simplification, if file was created with TFS_O_CREAT and there
    // is an error adding an entry to the open file table, the file is not
    // opened but it remains created
}

int tfs_sym_link(char const *target, char const *link_name) {
    inode_t * root_dir_inode = inode_get(ROOT_DIR_INUM);

    if (tfs_lookup(target, root_dir_inode) == -1) { // if file target doesn't exist
        return -1;
    }

    if (tfs_lookup(link_name, root_dir_inode) != -1) { // if link_name already exists
        return -1;
    }

    int fhandle = tfs_open(link_name, TFS_O_CREAT);
    if (fhandle == -1) {
        return -1;
    }

    int inumber_link = tfs_lookup(link_name, root_dir_inode);
    if (inumber_link == -1) {
        return -1;
    } 

    inode_t *inode_link = inode_get(inumber_link);
    inode_link->i_node_type = T_SYM_LINK; // change inode type to soft link

    if (inode_link->i_size == 0) {
        int bnum = data_block_alloc();
        if (bnum == -1) {
            return -1;
        }
        inode_link->i_data_block = bnum;
        char *block = data_block_get(bnum);
        strcpy(block, target);
    }

    if (tfs_close(fhandle) == -1) {
        return -1;
    }

    return 0;
}

int tfs_link(char const *target, char const *link_name) {
    inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);

    if (tfs_lookup(target, root_dir_inode) == -1) { // se não existir o ficheiro de destino
        return -1;
    }

    if (tfs_lookup(link_name, root_dir_inode) != -1) {
        return -1;
    }

    int inumber_target = tfs_lookup(target, root_dir_inode);
    if (inumber_target == -1) {
        return -1;
    }
    
    add_dir_entry(root_dir_inode, link_name + 1, inumber_target);
    inode_t *inode_target = inode_get(inumber_target);

    // cannot create hard link for soft link
    if (inode_target->i_node_type == T_SYM_LINK) {
        return -1;
    }

    inode_target->link_count++;

    return 0;
}

int tfs_close(int fhandle) {
    mutex_lock(&tfs_mutex);
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        mutex_unlock(&tfs_mutex);
        return -1; // invalid fd
    }

    remove_from_open_file_table(fhandle);
    mutex_unlock(&tfs_mutex);
    return 0;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t to_write) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }

    //  From the open file table entry, we get the inode
    inode_t *inode = inode_get(file->of_inumber);
    ALWAYS_ASSERT(inode != NULL, "tfs_write: inode of open file deleted");

    // Determine how many bytes to write
    size_t block_size = state_block_size();
    if (to_write + file->of_offset > block_size) {
        to_write = block_size - file->of_offset;
    }

    mutex_lock(&tfs_mutex);
    if (to_write > 0) {
        if (inode->i_size == 0) {
            // If empty file, allocate new block
            int bnum = data_block_alloc();
            if (bnum == -1) {
                mutex_unlock(&tfs_mutex);
                return -1; // no space
            }
            inode->i_data_block = bnum;
        }

        void *block = data_block_get(inode->i_data_block);
        ALWAYS_ASSERT(block != NULL, "tfs_write: data block deleted mid-write");

        // Perform the actual write
        memcpy(block + file->of_offset, buffer, to_write);

        // The offset associated with the file handle is incremented accordingly
        file->of_offset += to_write;
        if (file->of_offset > inode->i_size) {
            inode->i_size = file->of_offset;
        }
    }
    mutex_unlock(&tfs_mutex);
    return (ssize_t)to_write;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }

    // From the open file table entry, we get the inode
    inode_t const *inode = inode_get(file->of_inumber);
    ALWAYS_ASSERT(inode != NULL, "tfs_read: inode of open file deleted");

    // Determine how many bytes to read
    size_t to_read = inode->i_size - file->of_offset;
    if (to_read > len) {
        to_read = len;
    }

    if (to_read > 0) {
        void *block = data_block_get(inode->i_data_block);
        ALWAYS_ASSERT(block != NULL, "tfs_read: data block deleted mid-read");

        // Perform the actual read
        memcpy(buffer, block + file->of_offset, to_read);
        // The offset associated with the file handle is incremented accordingly
        file->of_offset += to_read;
    }

    return (ssize_t) to_read;
}

int tfs_unlink(char const *target) {
    inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM); 
    
    /* so it doesn't eliminate a file that is being opened and two threads 
    do not try to eliminate a file twice */
    mutex_lock(&tfs_mutex);
    int inumber = tfs_lookup(target, root_dir_inode);
    if (inumber == -1) {
    mutex_unlock(&tfs_mutex);
        return -1;
    }

    inode_t *inode = inode_get(inumber);
    inode->link_count--;

    if (clear_dir_entry(root_dir_inode, target + 1) == -1) {
        mutex_unlock(&tfs_mutex);
        return -1;
    }

    if (inode->link_count == 0) {
        if (inode->i_data_block != -1) {
            data_block_free(inode->i_data_block);
        }
        inode_delete(inumber);
    }
    mutex_unlock(&tfs_mutex);
    return 0;
}

int tfs_copy_from_external_fs(char const *source_path, char const *dest_path) {
    FILE *fp = fopen(source_path, "r");
    if (!fp) {
        return -1;
    }

    int fhandle = tfs_open(dest_path, TFS_O_CREAT | TFS_O_TRUNC);
    if (fhandle == -1) {
        return -1;
    }
    
    size_t len = 0;
    char buffer[1024];
    size_t sizeRead;
    ssize_t sizeWritten;

    while ((sizeRead = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0) {
        sizeWritten = tfs_write(fhandle, buffer, sizeRead);
        if (sizeWritten == -1) {
            return -1;
        }
        len += sizeRead;
    }

    if (len > state_block_size()) {
        tfs_unlink(dest_path);
        return -1;
    }

    if (tfs_close(fhandle) == -1 || fclose(fp) != 0) {
        return -1;
    }
    return 0;
}