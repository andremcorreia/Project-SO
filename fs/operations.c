//numeros alunos: 102666 | 103590
#include "operations.h"
#include "config.h"
#include "state.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "betterassert.h"

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
 */
static int tfs_lookup(char const *name, inode_t const *root_inode) {
	if (root_inode != inode_get(ROOT_DIR_INUM)  || !valid_pathname(name))
		return -1;
	// skip the initial '/' character
	name++;

	return find_in_dir(root_inode, name);
}

int tfs_open(char const *name, tfs_file_mode_t mode) {
	inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);

	pthread_rwlock_wrlock(&root_dir_inode->rw_lock);

	// Checks if the path name is valid
	if (!valid_pathname(name)){ 
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1;
	}

	LOCK_ASSERT_RW(root_dir_inode != NULL,
				  "tfs_open: root dir inode must exist",&root_dir_inode->rw_lock);
	int inum = tfs_lookup(name, root_dir_inode);
	size_t offset;

	if (inum >= 0) {
		// The file already exists
		inode_t *inode = inode_get(inum);

		pthread_rwlock_wrlock(&inode->rw_lock);

		LOCK_ASSERT_RW_RW(inode != NULL,
					  "tfs_open: directory files must have an inode",&inode->rw_lock,&root_dir_inode->rw_lock);

		// Truncate (if requested)
		if (mode & TFS_O_TRUNC) {
			if (inode->i_size > 0) {
				data_block_free(inode->i_data_block);
				inode->i_size = 0;
				inode->i_node_type = T_FILE; //if a symbolic link is truncated turns into a file
			}
		}

		// The file is a symbolic link, -1 if doesnt exist
		if(inode->i_node_type == T_SYM_LINK){
			int block = inode->i_data_block;
			pthread_rwlock_unlock(&inode->rw_lock);
			pthread_rwlock_unlock(&root_dir_inode->rw_lock);

			return tfs_open(data_block_get(block), mode);
		}

		// Determine initial offset
		if (mode & TFS_O_APPEND) {
			offset = inode->i_size;
		} else {
			offset = 0;
		}

		pthread_rwlock_unlock(&inode->rw_lock);

	} else if (mode & TFS_O_CREAT) {
		// The file does not exist; the mode specified that it should be created
		// Create inode
		inum = inode_create(T_FILE);
		if (inum == -1) {

			pthread_rwlock_unlock(&root_dir_inode->rw_lock);

			return -1; // no space in inode table
		}
		
		// Add entry in the root directory
		if (add_dir_entry(root_dir_inode, name + 1, inum) == -1) {
			inode_delete(inum);

			pthread_rwlock_unlock(&root_dir_inode->rw_lock);

			return -1; // no space in directory
		}

		offset = 0;
	} else {
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1;
	}

	// Finally, add entry to the open file table and return the corresponding
	// handle
	pthread_rwlock_unlock(&root_dir_inode->rw_lock);
	return add_to_open_file_table(inum, offset);

	// Note: for simplification, if file was created with TFS_O_CREAT and there
	// is an error adding an entry to the open file table, the file is not
	// opened but it remains created
}

int tfs_sym_link(char const *target, char const *link_name) {

	if (!valid_pathname(link_name))
		return -1; // invalid link name
	
	inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);
	pthread_rwlock_wrlock(&root_dir_inode->rw_lock);

	LOCK_ASSERT_RW(root_dir_inode != NULL, "tfs_sym_link: inode of root null\n",&root_dir_inode->rw_lock);

	if (tfs_lookup(link_name,root_dir_inode) != -1 || tfs_lookup(target,root_dir_inode) == -1){
		printf("Invalid name.\n");
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1; // tried to symbolically link to itself or to a non-existent file
	}

	int sym_inumber = inode_create(T_SYM_LINK);

	if (sym_inumber == -1){
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1; // failed to create
	}

	inode_t *sym_link = inode_get(sym_inumber);
	pthread_rwlock_wrlock(&sym_link->rw_lock);

	LOCK_ASSERT_RW_RW(sym_link != NULL, "tfs_sym_link: inode of sym link null",&sym_link->rw_lock,&root_dir_inode->rw_lock);

	sym_link->i_size = sizeof(target);

	int sym_b_num = data_block_alloc();
	if (sym_b_num == -1){ 
		pthread_rwlock_unlock(&sym_link->rw_lock);
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1; // no space
	}

	sym_link->i_data_block = sym_b_num;

	char* sym_block = data_block_get(sym_b_num);
	strcpy(sym_block, target);

	if (add_dir_entry(root_dir_inode, link_name + 1, sym_inumber) == -1){
		pthread_rwlock_unlock(&sym_link->rw_lock); 
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1; // no space in directory
	}

	pthread_rwlock_unlock(&sym_link->rw_lock);
	pthread_rwlock_unlock(&root_dir_inode->rw_lock);
	return 0;
}

int tfs_link(char const *target, char const *link_name) {
	
	inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);
	pthread_rwlock_rdlock(&root_dir_inode->rw_lock);

	LOCK_ASSERT_RW(root_dir_inode != NULL, "tfs_link: inode of root null\n",&root_dir_inode->rw_lock);

	

	if (tfs_lookup(link_name,root_dir_inode) != -1 || tfs_lookup(target,root_dir_inode) == -1){
		printf("Invalid name.\n");
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1; // tried to link to itself or to a non-existent file
	}

	int i_num_target = tfs_lookup(target, root_dir_inode);

	if (i_num_target == -1){
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1; // target not in root
	}

	inode_t *link = inode_get(i_num_target);

	pthread_rwlock_wrlock(&link->rw_lock);

	LOCK_ASSERT_RW_RW(link != NULL, "tfs_link: inode of link null\n",&link->rw_lock,&root_dir_inode->rw_lock);

	if(link->i_node_type == T_SYM_LINK || link == NULL){

		pthread_rwlock_unlock(&link->rw_lock);
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1; // target is null or a symbolic link
	}
		

	if (add_dir_entry(root_dir_inode, link_name + 1, i_num_target) == -1){

		pthread_rwlock_unlock(&link->rw_lock);
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1; // no space in directory
	}
		
	link->hard_link_counter++;

	pthread_rwlock_unlock(&link->rw_lock);
	pthread_rwlock_unlock(&root_dir_inode->rw_lock);

	return 0;
}

int tfs_close(int fhandle) {
	open_file_entry_t *file = get_open_file_entry(fhandle);
    pthread_mutex_lock(&file->of_lock);
	if (file == NULL){
        pthread_mutex_unlock(&file->of_lock); 
		return -1; // invalid fd
    }
	remove_from_open_file_table(fhandle);
	return 0;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t to_write) {
	open_file_entry_t *file = get_open_file_entry(fhandle);
	pthread_mutex_lock(&file->of_lock); 
	if (file == NULL) {
		pthread_mutex_unlock(&file->of_lock); 
		return -1;
	}
	
	//  From the open file table entry, we get the inode
	inode_t *inode = inode_get(file->of_inumber);
	pthread_rwlock_wrlock(&inode->rw_lock);

	LOCK_ASSERT_RW_MUTEX(inode != NULL, "tfs_write: inode of open file deleted\n",&inode->rw_lock,&file->of_lock);

	// Determine how many bytes to write
	size_t block_size = state_block_size();
	if (to_write + file->of_offset > block_size) 
		to_write = block_size - file->of_offset;

	if (to_write > 0) {
		if (inode->i_size == 0) {
			// If empty file, allocate new block
			int bnum = data_block_alloc();
			if (bnum == -1) {
				pthread_rwlock_unlock(&inode->rw_lock);
				pthread_mutex_unlock(&file->of_lock);
				return -1; // no space
			}

			inode->i_data_block = bnum;
		}

		void *block = data_block_get(inode->i_data_block);
		
		LOCK_ASSERT_RW_MUTEX(block != NULL, "tfs_write: data block deleted mid-write\n",&inode->rw_lock,&file->of_lock);

		// Perform the actual write
		memcpy(block + file->of_offset, buffer, to_write);

		// The offset associated with the file handle is incremented accordingly
		file->of_offset += to_write;
		if (file->of_offset > inode->i_size) 
			inode->i_size = file->of_offset;
	}   

	pthread_rwlock_unlock(&inode->rw_lock);
	pthread_mutex_unlock(&file->of_lock); 
    return (ssize_t)to_write;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
	open_file_entry_t *file = get_open_file_entry(fhandle);
	pthread_mutex_lock(&file->of_lock); 
	if (file == NULL) {
		pthread_mutex_unlock(&file->of_lock); 
		return -1;
	}

	// From the open file table entry, we get the inode
	inode_t *inode = inode_get(file->of_inumber);
	pthread_rwlock_rdlock(&inode->rw_lock);

	LOCK_ASSERT_RW_MUTEX(inode != NULL, "tfs_read: inode of open file deleted\n",&inode->rw_lock,&file->of_lock);

	// Determine how many bytes to read
	size_t to_read = inode->i_size - file->of_offset;
	if (to_read > len) 
		to_read = len;

	if (to_read > 0) {
		void *block = data_block_get(inode->i_data_block);
		
		LOCK_ASSERT_RW_MUTEX(block != NULL, "tfs_read: data block deleted mid-read\n",&inode->rw_lock,&file->of_lock);

		// Perform the actual read
		memcpy(buffer, block + file->of_offset, to_read);
		// The offset associated with the file handle is incremented accordingly
		file->of_offset += to_read;
	}
	pthread_rwlock_unlock(&inode->rw_lock);
    pthread_mutex_unlock(&file->of_lock); 
	return (ssize_t)to_read;
}

int tfs_unlink(char const *target) {
	inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);

	pthread_rwlock_wrlock(&root_dir_inode->rw_lock);
	
	LOCK_ASSERT_RW(root_dir_inode != NULL, "tfs_unlink: inode of root null\n",&root_dir_inode->rw_lock);
	
	int i_num_target = tfs_lookup(target, root_dir_inode);
	
	if(i_num_target == -1){
		pthread_rwlock_unlock(&root_dir_inode->rw_lock);
		return -1; // target not in root
	}

	inode_t *target_node = inode_get(i_num_target);

	pthread_rwlock_wrlock(&target_node->rw_lock);

	LOCK_ASSERT_RW_RW(target_node != NULL, "tfs_unlink: inode of target null\n",&target_node->rw_lock,&root_dir_inode->rw_lock);

	// Unlink a Symbolic link
	if(target_node->i_node_type == T_SYM_LINK){
		inode_delete(i_num_target);
	}
	else{  // Unlink a file
		target_node->hard_link_counter--;   //Remove the last hard link from the file in order to delete it
		if(target_node->hard_link_counter == 0){
			if(check_if_open(i_num_target) == 0){
				pthread_rwlock_unlock(&target_node->rw_lock);
				pthread_rwlock_unlock(&root_dir_inode->rw_lock);
				return -1;
			}		// file is open so it cannot be unlinked
			inode_delete(i_num_target);
		}
	}    
	int success_bool = clear_dir_entry(root_dir_inode, target+1);
	pthread_rwlock_unlock(&target_node->rw_lock);
	pthread_rwlock_unlock(&root_dir_inode->rw_lock);

	return success_bool;
}

int tfs_copy_from_external_fs(char const *source_path, char const *dest_path) {

	FILE* fd = fopen(source_path, "r");
	int fdo = tfs_open(dest_path, TFS_O_TRUNC | TFS_O_CREAT);
	if (!fd || fdo == -1)
		return -1;
	
	char buffer[128];
	memset(buffer,0,sizeof(buffer));

	size_t bytes_read = 0;
	while((bytes_read = fread(buffer, sizeof(char), sizeof(buffer), fd)))
	{
		ssize_t bytes_written = tfs_write(fdo, buffer, bytes_read);
		if (bytes_written < 0){
			fclose(fd);
			tfs_close(fdo);
			return -1;
		}
	}

	fclose(fd);
	tfs_close(fdo);
	return 0;
}