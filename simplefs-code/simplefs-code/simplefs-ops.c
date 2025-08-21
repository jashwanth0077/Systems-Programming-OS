#include "simplefs-ops.h"
extern struct filehandle_t file_handle_array[MAX_OPEN_FILES]; // Array for storing opened files

int simplefs_create(char *filename){
    /*
	    Create file with name `filename` from disk
	*/
	for (int i = 0; i < MAX_OPEN_FILES; i++) {
		if (file_handle_array[i].inode_number > 0) {
			struct inode_t *inode = (struct inode_t*)malloc(sizeof(struct inode_t*));
			simplefs_readInode(file_handle_array[i].inode_number, inode);
			if (inode->name == filename) {
				return -1;
			}
			else {
				continue;
			}
		}
	}
	for (int i = 0; i < NUM_INODES; i++) {
		struct inode_t *inode;
		simplefs_readInode(i, inode);
		if (inode->name == filename) {
			return -1;
		}
		else {
			continue;
		}
	}
	int index = simplefs_allocInode();
	if (index == -1) {
		return -1;
	}
	struct inode_t *inode;
	simplefs_readInode(index, inode);
	inode->status = INODE_IN_USE;
	memcpy(inode->name, filename, strlen(filename)+1);
	inode->file_size = 0;
	for (int i = 0; i < MAX_FILE_SIZE; i++) {
		inode->direct_blocks[i] = -1;
	}
	simplefs_writeInode(index, inode);
    return index;
	// for(int i = 0; i < NUM_INODES; i++){
	// 	struct inode_t inode;
	// 	simplefs_readInode(i, &inode);
	// 	if((inode.status == INODE_IN_USE) && (strcmp(inode.name, filename) == 0)){
	// 		return -1;
	// 	}
	// }
	// int ret_inode = simplefs_allocInode();
	// if(ret_inode == -1){
	// 	return -1;
	// }
	
	// struct inode_t inode;
	// simplefs_readInode(ret_inode, &inode);
	// inode.file_size = 0;
	// inode.status = INODE_IN_USE;
	// memset(inode.name, 0, MAX_NAME_STRLEN);
	// strncpy(inode.name, filename, MAX_NAME_STRLEN - 1);
	// simplefs_writeInode(ret_inode, &inode);
    // return ret_inode;
}


void simplefs_delete(char *filename){
    /*
	    delete file with name `filename` from disk
	*/
	for (int i= 0; i < NUM_INODES; i++) {
		struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
		simplefs_readInode(i, inode);
		// return;
		printf("%d\n",i);
		if (strcmp(inode->name, filename) == 0 && (inode->status == INODE_IN_USE)) {
			inode->status = INODE_FREE;
			for (int j = 0; j < MAX_FILE_SIZE; j++) {
				if (inode->direct_blocks[j] != -1) {
					simplefs_freeDataBlock(inode->direct_blocks[j]);
				}
			}
			simplefs_freeInode(i);
		}
		break;
	}
	return;
	// int found = -1;
	// for(int i = 0; i < NUM_INODES; i++){
	// 	struct inode_t inode;
	// 	simplefs_readInode(i, &inode);
	// 	if((inode.status == INODE_IN_USE) && (strcmp(inode.name, filename) == 0)){
	// 		found = i;
	// 		break;
	// 	}
	// }
	// if(found == -1){
	// 	return;
	// }
	// struct inode_t inode;
	// simplefs_readInode(found, &inode);
	// for(int i = 0; i < MAX_FILE_SIZE; i++){
	// 	if(inode.direct_blocks[i] != -1){
	// 		simplefs_freeDataBlock(inode.direct_blocks[i]);
	// 	}
	// }
	// simplefs_freeInode(found);
}

int simplefs_open(char *filename){
    /*
	    open file with name `filename`
	*/
	int index;
	for (int i = 0; i < NUM_INODES; i++) {
		struct inode_t* inode; //overwriting causing errors!!!
		simplefs_readInode(i, inode);
		if (strcmp(inode->name, filename) == 0) {
			index = i;
			break;
		}
		if (i == NUM_INODES-1) {
			return -1;
		}
	}
	int handle;
	for (int j = 0; j < MAX_OPEN_FILES; j++) {
		if (file_handle_array[j].inode_number < 0) {
			file_handle_array[j].inode_number = index;
			file_handle_array[j].offset = 0;
			handle = j;
			break;
		}
		else {
			continue;
		}
	}
    return handle;
	// int found = -1;
	// for(int i = 0; i < NUM_INODES; i++){
	// 	struct inode_t inode;
	// 	simplefs_readInode(i, &inode);
	// 	if((inode.status == INODE_IN_USE) && (strcmp(inode.name, filename) == 0)){
	// 		found = i;
	// 		break;
	// 	}
	// }
	// if(found == -1){
	// 	return -1;
	// }
	// int handle = -1;
	// for(int i = 0; i < MAX_OPEN_FILES; i++){
	// 	if(file_handle_array[i].inode_number < 0){
	// 		handle = i;
	// 		file_handle_array[i].inode_number = found;
	// 		file_handle_array[i].offset = 0;
	// 		break;
	// 	}
	// }
    // return handle;
}

void simplefs_close(int file_handle){
    /*
	    close file pointed by `file_handle`
	*/
	file_handle_array[file_handle].inode_number = -1;
	file_handle_array[file_handle].offset = 0;
}

int simplefs_read(int file_handle, char *buf, int nbytes){
    /*
	    read `nbytes` of data into `buf` from file pointed by `file_handle` starting at current offset
	*/
	struct inode_t* inode = (struct inode_t*)malloc(sizeof(struct inode_t*));
	int inode_num = file_handle_array[file_handle].inode_number;
	int offset = file_handle_array[file_handle].offset;
	simplefs_readInode(inode_num, inode);
	int index = offset/BLOCKSIZE;
	int off = offset%BLOCKSIZE;
	int data_read = 0;
	if (offset + nbytes > inode->file_size) {
		return -1;
	}
	else {
		for (int i = 0; i < MAX_FILE_SIZE; i++) {
			if (inode->direct_blocks[i] != -1) {
				return -1;
			}
			char ex[BLOCKSIZE];
			simplefs_readDataBlock(inode->direct_blocks[i], ex);
			if (i = index) {
				if (nbytes > BLOCKSIZE-off) {
					memcpy(buf ,ex+off, BLOCKSIZE-off);
					data_read += BLOCKSIZE-off;
				}
				else {
					memcpy(buf ,ex+off, nbytes);
					data_read += nbytes;
					return 0;
				}
			}
			else if (i > index) {
				if (nbytes-data_read > BLOCKSIZE) {
					memcpy(buf+data_read ,ex, BLOCKSIZE);
					data_read += BLOCKSIZE;
				}
				else {
					memcpy(buf+data_read ,ex, nbytes-data_read);
					data_read += nbytes-data_read;
					return 0;
				} 
			}
		}
	}
	return 0;
}


int simplefs_write(int file_handle, char *buf, int nbytes){
    /*
	    write `nbytes` of data from `buf` to file pointed by `file_handle` starting at current offset
	*/
	struct filehandle_t *fh = &(file_handle_array[file_handle]);
	if(fh->inode_number < 0){
		return -1;
	}
	struct inode_t inode;
	simplefs_readInode(fh->inode_number, &inode);
	if(inode.status == INODE_FREE){
		return -1;
	}
	if(nbytes + fh->offset > MAX_FILE_SIZE * BLOCKSIZE){
		return -1;
	}
	int num_blks = 0;
	for (int i = 0; i < MAX_FILE_SIZE; i++)
	{
		if(inode.direct_blocks[i] != -1){
			num_blks++;
		}
	}
	if((fh->offset % BLOCKSIZE == 0) && (fh->offset / BLOCKSIZE < MAX_FILE_SIZE) && (fh->offset == inode.file_size) && (inode.direct_blocks[fh->offset / BLOCKSIZE] == -1)){
		inode.direct_blocks[num_blks] = simplefs_allocDataBlock();
		if(inode.direct_blocks[num_blks] == -1){
			return -1;
		}
		char nulldata[BLOCKSIZE];
		memset(nulldata, 0, BLOCKSIZE);
		simplefs_writeDataBlock(inode.direct_blocks[num_blks], nulldata);
		num_blks++;
	}

	int blk = fh->offset / BLOCKSIZE;
	int off_in_blk = fh->offset % BLOCKSIZE;
	int nbytes_avail = (num_blks - (blk + 1)) * BLOCKSIZE + (BLOCKSIZE - off_in_blk);
	int nbytes_req = (nbytes - nbytes_avail > 0) ? nbytes - nbytes_avail : 0;
	int nblocks_req = nbytes_req/BLOCKSIZE + (nbytes_req % BLOCKSIZE ? 1 : 0);
	int new_blocks[nblocks_req];
	for(int i = 0; i < nblocks_req; i++){
		new_blocks[i] = -1;
	}
	for(int i = 0; i < nblocks_req; i++){
		new_blocks[i] = simplefs_allocDataBlock();
		if(new_blocks[i] < 0){
			goto bad;
		}
	}
	goto good;
bad:
	for(int i = 0; i < nblocks_req; i++){
		if(new_blocks[i] >= 0){
			simplefs_freeDataBlock(new_blocks[i]);
		}
	}
	return -1;
good:
	for(int i = 0; i < nblocks_req; i++){
		char nulldata[BLOCKSIZE];
		memset(nulldata, 0, BLOCKSIZE);
		simplefs_writeDataBlock(new_blocks[i], nulldata);
	}

	assert(nblocks_req + num_blks <= MAX_FILE_SIZE);
	for(int i = num_blks; i < nblocks_req + num_blks; i++){
		inode.direct_blocks[i] = new_blocks[i - num_blks];
	}
	int bytes_left_to_write = nbytes;
	while((blk < MAX_FILE_SIZE) && (bytes_left_to_write > 0)){
		char blk_data[BLOCKSIZE];
		simplefs_readDataBlock(inode.direct_blocks[blk], blk_data);
		if(off_in_blk + bytes_left_to_write > BLOCKSIZE){
			memcpy(blk_data + off_in_blk, buf + (nbytes - bytes_left_to_write) , BLOCKSIZE - off_in_blk);
			simplefs_writeDataBlock(inode.direct_blocks[blk], blk_data);
			blk++;
			bytes_left_to_write -= (BLOCKSIZE - off_in_blk);
			off_in_blk = 0;
		}
		else{
			memcpy(blk_data + off_in_blk, buf + (nbytes - bytes_left_to_write), bytes_left_to_write);
			simplefs_writeDataBlock(inode.direct_blocks[blk], blk_data);
			bytes_left_to_write = 0;
			break;
		}
	}
	inode.file_size = (fh->offset + nbytes > inode.file_size) ? (fh->offset + nbytes) : inode.file_size;
	simplefs_writeInode(fh->inode_number, &inode);
    return 0;
    // return -1;
}


int simplefs_seek(int file_handle, int nseek){
    /*
	   increase `file_handle` offset by `nseek`
	*/
	struct inode_t* inode;
	if (file_handle_array[file_handle].inode_number < 0) {
		return -1;
	}
	simplefs_readInode(file_handle_array[file_handle].inode_number, inode);
	if (inode->status == INODE_FREE) {
		return -1;
	}
	if (nseek > 0) {
		if (file_handle_array[file_handle].offset + nseek > inode->file_size) {
			file_handle_array[file_handle].offset = inode->file_size;
		}
		else {
			file_handle_array[file_handle].offset = file_handle_array[file_handle].offset + nseek;
		}
	}
	else if (nseek < 0) {
		if (file_handle_array[file_handle].offset + nseek < 0) {
			file_handle_array[file_handle].offset = 0;
		}
		else {
			file_handle_array[file_handle].offset = file_handle_array[file_handle].offset + nseek;
		}
	}
    return 0;
}