#include "../lib/operations.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int find_free_block(file_system *fs);


int
fs_mkdir(file_system *fs, char *path)
{
	if (fs == NULL || path == NULL) {
        printf("Error: Null argument(s) provided.\n");
        return -1;
    }

    if (path[0] != '/') {
        printf("Invalid directory name. Directory name should start with a slash.\n");
        return -1;
    }

    char *path_copy = strdup(path);
    char *token = strtok(path_copy, "/");
    if (token == NULL) {
        printf("Directory name is empty.\n");
        return -1;
    }

    int parent_inode_index = fs->root_node;
    inode *parent_inode = &fs->inodes[parent_inode_index];

    while (token != NULL) {
        char *next_token = strtok(NULL, "/");

        if (next_token == NULL) {
        for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
            int child_inode_index = parent_inode->direct_blocks[i];
            if (child_inode_index != -1 && strcmp(fs->inodes[child_inode_index].name, token) == 0) {
                if (fs->inodes[child_inode_index].n_type == reg_file) {
                    printf("File with the same name already exists, but allowing the creation of a directory with the same name.\n");
                    // Don't return an error here, continue with the directory creation
                } else {
                    printf("Directory with the same name already exists.\n");
                    return -1;
                }
            }
        }
        break;
    }

        int found = 0;
        for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
            int child_inode_index = parent_inode->direct_blocks[i];
            if (child_inode_index != -1 
                && fs->inodes[child_inode_index].n_type == directory
                && strcmp(fs->inodes[child_inode_index].name, token) == 0) {
                parent_inode_index = child_inode_index;
                parent_inode = &fs->inodes[parent_inode_index];
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Parent directory does not exist.\n");
            return -1;
        }

        token = next_token;
    }

    int new_inode_index = find_free_inode(fs);
    if (new_inode_index == -1) {
        printf("No free INodes available.\n");
        return -1;
    }

    inode *new_inode = &fs->inodes[new_inode_index];
    new_inode->n_type = directory;
    strncpy(new_inode->name, token, NAME_MAX_LENGTH);
    new_inode->parent = parent_inode_index;
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        new_inode->direct_blocks[i] = -1;
    }

    int found = 0;
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        if (parent_inode->direct_blocks[i] == -1) {
            parent_inode->direct_blocks[i] = new_inode_index;
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Parent directory's direct blocks are full.\n");
        return -1;
    }

    fs->free_list[new_inode_index] = 0;
    fs->s_block->free_blocks--;

    free(path_copy);

    return 0;
}

int
fs_mkfile(file_system *fs, char *path_and_name)
{
	if (fs == NULL || path_and_name == NULL || path_and_name[0] != '/') {
        printf("Invalid argument(s). File name should start with a slash and arguments must not be NULL.\n");
        return -1;
    }

    char *path_copy = strdup(path_and_name);
    char *token = strtok(path_copy, "/");
    if (token == NULL) {
        printf("File name is empty.\n");
        return -1;
    }

    int parent_inode_index = fs->root_node;
    inode *parent_inode = &fs->inodes[parent_inode_index];

    while (token != NULL) {
        char *next_token = strtok(NULL, "/");
        if (next_token == NULL) {
            break;
        }

        int found = 0;
        for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
            int child_inode_index = parent_inode->direct_blocks[i];
            if (child_inode_index != -1 && strcmp(fs->inodes[child_inode_index].name, token) == 0) {
                if (fs->inodes[child_inode_index].n_type == directory) {
                    parent_inode_index = child_inode_index;
                    parent_inode = &fs->inodes[parent_inode_index];
                    found = 1;
                    break;
                }
            }
        }

        if (!found) {
            printf("Parent directory does not exist.\n");
            free(path_copy);
            return -1;
        }

        token = next_token;
    }

    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        int child_inode_index = parent_inode->direct_blocks[i];
        if (child_inode_index != -1 && strcmp(fs->inodes[child_inode_index].name, token) == 0) {
            // If a directory already exists with the same name, allow the file creation.
            if (fs->inodes[child_inode_index].n_type == directory) {
                continue;
            }
            printf("A file with the same name already exists in this directory.\n");
            free(path_copy);
            return -2;
        }
    }

    int new_inode_index = find_free_inode(fs);
    if (new_inode_index == -1) {
        printf("No free INodes available.\n");
        free(path_copy);
        return -1;
    }

    inode *new_inode = &fs->inodes[new_inode_index];
    new_inode->n_type = reg_file;
    strncpy(new_inode->name, token, NAME_MAX_LENGTH);
    new_inode->parent = parent_inode_index;
    new_inode->size = 0;
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        new_inode->direct_blocks[i] = -1;
    }

    int dir_inserted = 0;
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        if (parent_inode->direct_blocks[i] == -1) {
            parent_inode->direct_blocks[i] = new_inode_index;
            dir_inserted = 1;
            break;
        }
    }

    free(path_copy);

    if (!dir_inserted) {
        printf("Parent directory is full.\n");
        return -1;
    }

    fs->free_list[new_inode_index] = 0;
    fs->s_block->free_blocks--;

    return 0;
}

char *
fs_list(file_system *fs, char *path)
{
	// Check if fs or path is NULL
    if (fs == NULL || path == NULL) {
        printf("Invalid argument. fs and path should not be NULL.\n");
        return NULL;
    }
    
    // Check if the path starts with a slash
    if (path[0] != '/') {
        printf("Invalid directory name. Directory name should start with a slash.\n");
        return NULL;
    }

    // Split the path into tokens using slash as the delimiter
    char *path_copy = strdup(path);
    char *token = strtok(path_copy, "/");

    // Start from the root inode
    int parent_inode_index = fs->root_node;
    inode *parent_inode = &fs->inodes[parent_inode_index];

    // Find the directory
    while (token != NULL) {
        char *next_token = strtok(NULL, "/");

        // Look for the next token in the parent directory's direct blocks
        int found = 0;
        for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
            int child_inode_index = parent_inode->direct_blocks[i];
            if (child_inode_index != -1 && strcmp(fs->inodes[child_inode_index].name, token) == 0) {
                // Found the next directory in the path
                parent_inode_index = child_inode_index;
                parent_inode = &fs->inodes[parent_inode_index];
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Directory does not exist.\n");
            return NULL;
        }

        token = next_token;
    }

    // Create a list to store the inode indices of the files and directories
    int inode_indices[DIRECT_BLOCKS_COUNT];
    int count = 0;

    // Add the inode indices of the files and directories in the parent directory's direct blocks to the list
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        int child_inode_index = parent_inode->direct_blocks[i];
        if (child_inode_index != -1) {
            inode_indices[count++] = child_inode_index;
        }
    }

    // Sort the list of inode indices
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (inode_indices[j] < inode_indices[i]) {
                int temp = inode_indices[i];
                inode_indices[i] = inode_indices[j];
                inode_indices[j] = temp;
            }
        }
    }

    // Create a string to store the list of files and directories
    char *list = malloc(DIRECT_BLOCKS_COUNT * (NAME_MAX_LENGTH + 5)); // Extra space for "DIR " or "FIL " and newline
    list[0] = '\0';

    // Add the names of the files and directories to the list in the sorted order
    for (int i = 0; i < count; i++) {
        inode *child_inode = &fs->inodes[inode_indices[i]];
        char *type = (child_inode->n_type == directory) ? "DIR " : "FIL ";
        strcat(list, type);
        strcat(list, child_inode->name);
        strcat(list, "\n");
    }

    free(path_copy);

    return list;
}

int
fs_writef(file_system *fs, char *filename, char *text)
{
    // Check if fs, filename, or text is NULL
    if (fs == NULL || filename == NULL || text == NULL) {
        printf("Invalid argument. fs, filename, and text should not be NULL.\n");
        return -1;
    }

    // If text is an empty string, return 0 without any further operations
    if (strlen(text) == 0) {
        return 0;
    }

    // Extract the file name from the file path
    char *file_name = strrchr(filename, '/');
    if (file_name != NULL) {
        file_name++; // Skip the '/'
    } else {
        file_name = filename;
    }

    // Traverse the filesystem in reverse order to find the file
    for (int i = fs->s_block->num_blocks - 1; i >= 0; i--) {
        inode *node = &fs->inodes[i];
        if (node->n_type == reg_file && strcmp(node->name, file_name) == 0) {
            // Found the file, now append the text to it
            int text_len = strlen(text);
            int text_pos = 0;
            for (int j = 0; j < DIRECT_BLOCKS_COUNT; j++) {
                int block_index = node->direct_blocks[j];
                if (text_pos < text_len && block_index == -1) {
                    // No block associated with this index, find a free block
                    block_index = find_free_block(fs);
                    if (block_index == -1) {
                        return -2; // No free blocks
                    }
                    node->direct_blocks[j] = block_index;
                    // Update the free list
                    fs->free_list[block_index] = 0;
                    // Update the superblock
                    fs->s_block->free_blocks--;
                }
                if (block_index != -1) {
                    data_block *block = &fs->data_blocks[block_index];
                    int space_left = BLOCK_SIZE - block->size;
                    int to_write = text_len - text_pos < space_left ? text_len - text_pos : space_left;
                    memcpy(block->block + block->size, text + text_pos, to_write);
                    block->size += to_write;
                    node->size += to_write;
                    text_pos += to_write;
                    if (text_pos == text_len) {
                        return text_len; // All text written
                    }
                }
            }
            return -2; // Not enough space in the file's blocks to append the text
        }
    }

    // File not found
    return -1;
}

uint8_t *
fs_readf(file_system *fs, char *filename, int *file_size)
{
	// Check if fs, filename, or file_size is NULL
    if (fs == NULL || filename == NULL || file_size == NULL) {
        printf("Invalid argument. fs, filename, and file_size should not be NULL.\n");
        return NULL;
    }
    
    // Extract the file name from the file path
    char *file_name = strrchr(filename, '/');
    if (file_name != NULL) {
        file_name++; // Skip the '/'
    } else {
        file_name = filename;
    }

    // Traverse the filesystem in reverse order to find the file
    for (int i = fs->s_block->num_blocks - 1; i >= 0; i--) {
        inode *node = &fs->inodes[i];
        if (node->n_type == reg_file && strcmp(node->name, file_name) == 0) {
            // Found the file, now read its content
            *file_size = 0;
            for (int j = 0; j < DIRECT_BLOCKS_COUNT; j++) {
                int block_index = node->direct_blocks[j];
                if (block_index != -1) {
                    data_block *block = &fs->data_blocks[block_index];
                    *file_size += block->size;
                }
            }
            if (*file_size == 0) {
                return NULL; // File is empty
            }
            uint8_t *file_content = (uint8_t *)malloc(*file_size);
            int file_pos = 0;
            for (int j = 0; j < DIRECT_BLOCKS_COUNT; j++) {
                int block_index = node->direct_blocks[j];
                if (block_index != -1) {
                    data_block *block = &fs->data_blocks[block_index];
                    memcpy(file_content + file_pos, block->block, block->size);
                    file_pos += block->size;
                }
            }
            return file_content;
        }
    }
    // File not found
    return NULL;
}


int
fs_rm(file_system *fs, char *path)
{
	// Check if fs or path is NULL
    if (fs == NULL || path == NULL) {
        printf("Invalid argument. fs and path should not be NULL.\n");
        return -1;
    }

    // Extract the file name from the path
    char *file_name = strrchr(path, '/');
    if (file_name != NULL) {
        file_name++; // Skip the '/'
    } else {
        file_name = path;
    }

    // Traverse the filesystem in reverse order to find the file or directory
    int file_found = 0;
    for (int i = fs->s_block->num_blocks - 1; i >= 0; i--) {
        inode *node = &fs->inodes[i];
        if ((node->n_type == reg_file || node->n_type == directory) && strcmp(node->name, file_name) == 0) {
            // Check if it's a root file or directory
            if (node->parent == 0 && path[0] != '/') {
                continue;  // If it's a root file or directory but the path does not start with '/', skip it
            }
            // Found the file or directory, now delete it
            file_found = 1;
            if (node->n_type == directory) {
                // If it's a directory, recursively delete all its contents
                for (int j = 0; j < DIRECT_BLOCKS_COUNT; j++) {
                    int child_inode_index = node->direct_blocks[j];
                    if (child_inode_index != -1) {
                        // Construct the child's path
                        char child_path[NAME_MAX_LENGTH + 1];
                        snprintf(child_path, sizeof(child_path), "%s/%s", path, fs->inodes[child_inode_index].name);
                        // Recursively delete the child
                        fs_rm(fs, child_path);
                    }
                }
            }
            // Delete the file or directory itself
            for (int j = 0; j < DIRECT_BLOCKS_COUNT; j++) {
                int block_index = node->direct_blocks[j];
                if (block_index != -1) {
                    // Free the data block
                    fs->free_list[block_index] = 1;
                    node->direct_blocks[j] = -1;
                    fs->s_block->free_blocks += 1;  // Increase the number of free blocks
                }
            }
            // Mark the inode as free
            node->n_type = free_block;
            // Update the parent inode
            inode *parent_node = &fs->inodes[node->parent];
            for (int j = 0; j < DIRECT_BLOCKS_COUNT; j++) {
                if (parent_node->direct_blocks[j] == i) {
                    parent_node->direct_blocks[j] = -1;
                    break;
                }
            }
        }
    }
    // File or directory not found
    if (file_found == 0) {
        return -1;
    }

    return 0; // Success
}

int
fs_import(file_system *fs, char *int_path, char *ext_path)
{
    // Check if fs, int_path, or ext_path is NULL
    if (fs == NULL || int_path == NULL || ext_path == NULL) {
        printf("Invalid argument. fs, int_path, and ext_path should not be NULL.\n");
        return -1;
    }
    
    // Open the external file for reading
    FILE *ext_file = fopen(ext_path, "rb");
    if (ext_file == NULL) {
        perror("Failed to open external file");
        return -1;
    }
    printf("Opened external file: %s\n", ext_path);  // Print statement

    // Find the internal file
    char *file_name = strrchr(int_path, '/');
    if (file_name != NULL) {
        file_name++; // Skip the '/'
    } else {
        file_name = int_path;
    }
    inode *int_file = NULL;
    for (int i = 0; i < fs->s_block->num_blocks; i++) {
        if (fs->inodes[i].n_type == reg_file && strcmp(fs->inodes[i].name, file_name) == 0) {
            int_file = &fs->inodes[i];
            break;
        }
    }
    if (int_file == NULL) {
        fprintf(stderr, "Internal file not found\n");
        fclose(ext_file);
        return -1;
    }

    // Read the external file and write its contents to the internal file
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        int block_index = int_file->direct_blocks[i];
        if (block_index == -1) {
            // Find a free block and allocate it to the file
            block_index = find_free_block(fs);
            if (block_index == -1) {
                fprintf(stderr, "No free blocks available\n");
                break;
            }
            int_file->direct_blocks[i] = block_index;
            fs->free_list[block_index] = 0;  // Update the free list
        }

        // Now block_index should be a valid block index
        size_t bytes_read = fread(fs->data_blocks[block_index].block, 1, BLOCK_SIZE, ext_file);
        fs->data_blocks[block_index].size = bytes_read;
        int_file->size += bytes_read;
        if (bytes_read < BLOCK_SIZE) {
            // End of file reached
            break;
        }
    }


    // Close the external file
    fclose(ext_file);
    printf("Closed external file\n");  // Print statement

    return 0; // Success
}

int
fs_export(file_system *fs, char *int_path, char *ext_path)
{
	// Check if fs, int_path, or ext_path is NULL
    if (fs == NULL || int_path == NULL || ext_path == NULL) {
        printf("Invalid argument. fs, int_path, and ext_path should not be NULL.\n");
        return -1;
    }

    // Extract the file name from the path
    char *file_name = strrchr(int_path, '/');
    if (file_name != NULL) {
        file_name++; // Skip the '/'
    } else {
        file_name = int_path;
    }

    // Traverse the filesystem in reverse order to find the file
    for (int i = fs->s_block->num_blocks - 1; i >= 0; i--) {
        inode *node = &fs->inodes[i];
        if (node->n_type == reg_file && strcmp(node->name, file_name) == 0) {
            // Found the file, now export its content
            FILE *ext_file = fopen(ext_path, "wb");
            if (ext_file == NULL) {
                perror("Failed to open external file");
                printf("Attempted to open file at path: %s\n", ext_path);
                return -1;
            }
            for (int j = 0; j < DIRECT_BLOCKS_COUNT; j++) {
                int block_index = node->direct_blocks[j];
                if (block_index != -1) {
                    data_block *block = &fs->data_blocks[block_index];
                    fwrite(block->block, 1, block->size, ext_file);
                }
            }
            fclose(ext_file);
            return 0; // Success
        }
    }
    // File not found
    printf("Internal file not found: %s\n", int_path);
    return -1;
}


// helper function used: writef and import
int find_free_block(file_system *fs) {
    for (int i = 0; i < fs->s_block->num_blocks; i++) {
        if (fs->data_blocks[i].size == 0) {
            return i; // Found a free block
        }
    }
    return -1; // No free block found
}

