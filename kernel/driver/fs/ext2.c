#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int32_t i32;
typedef int16_t i16;
#include <kernel/driver/ext2.h>

#define alloc(size) malloc(size)

#define INODE_BG(fs, inode_id)  ((inode_id - 1) / fs->fs_info->inodes_per_group)
#define BLOCK_OFFSET(fs, block) (block * fs->fs_info->block_size)

size_t read_fs(fs_t *fs, void *restrict buffer, u64 offset, u64 length) {
    return 0;
}

size_t write_fs(fs_t *fs, const void *buffer, u64 offset, u64 length) {
    return 0;
}

int write_superblock(ext2_fs_t *fs) {
    // TODO verify sb exists
    // Write only the first 264 bytes corresponding to disk data and not memory
    // data
    return write_fs((fs_t *)fs, fs->fs_info, 1024, 264) == 0;
}

int mount(FILE *img, ext2_fs_t *fs) {
    int rc = 0;
    fs->fs_img = img;

    ext2_sb_info_t *sb = (ext2_sb_info_t *)alloc(sizeof(ext2_sb_info_t));
    read_fs((fs_t *)fs, (void *)sb, 1024L, sizeof(ext2_sb_info_t));

    // Verify fs is a valid ext2 fs and can be mounted
    if (sb->magic != EXT2_MAGIC) return -1;
    if (sb->state != EXT2_VALID_FS) {
        switch (sb->errors) {
            case EXT2_ERRORS_RO:
                rc = 1;
                sb->readonly = 1;
                break;
            case EXT2_ERRORS_PANIC:
                // TODO kernel panic
                return -1;
            case EXT2_ERRORS_CONTINUE:
            default:
                break;
        }
    }

    if (sb->rev_level >= 1) {
        // testing incompat features
        u32 not_supported = EXT2_FEATURE_INCOMPAT_COMPRESSION
                            | EXT2_FEATURE_INCOMPAT_JOURNAL_DEV
                            | EXT2_FEATURE_INCOMPAT_META_BG;
        if ((sb->feature_incompat & not_supported) != 0) {
            // some features needed by the fs are not currently supported
            return 1;
        }

        // testing ro_compat features
        not_supported = EXT2_FEATURE_RO_COMPAT_BTREE_DIR;
        if ((sb->feature_ro_compat & not_supported) != 0) {
            // some features are not supported fs need to be mounted in
            // read-only mode
            rc = 1;
            sb->readonly = 1;
        }
    }

    time_t now = time(NULL);

    // Verify if any fs consistency check is needed
    if (sb->max_mnt_count > 0 && sb->mnt_count > sb->max_mnt_count) {
        // TODO verify fs
    }

    if (sb->checkinterval > 0 && sb->mtime + sb->checkinterval > now) {
        // TODO verify fs
    }

    sb->block_size = 1024 << sb->log_block_size;
    sb->mnt_count++;
    sb->mtime = now;
    sb->wtime = now;
    sb->state = EXT2_UNCLEAN_FS;
    fs->type = EXT2;
    fs->fs_info = sb;
    if (write_superblock(fs)) {
        return 1;
    }

    return rc;
}

int umount(ext2_fs_t *fs) {
    // TODO check to verify fs is mounted

    ext2_superblock_t *sb = (ext2_superblock_t *)fs->fs_info;
    sb->state = EXT2_VALID_FS;
    write_superblock(fs);
    free(sb);
    return 0;
}

ext2_bg_desc_t get_bg_desc(ext2_fs_t *fs, u32 id) {
    // Table start at block 2 if block size is 1024 bytes else block 1
    u32 bgd_table_start = 1;
    if (fs->fs_info->block_size == 1024) bgd_table_start = 2;

    u64 offset = (bgd_table_start * fs->fs_info->block_size)
                 + (id * sizeof(ext2_bg_desc_t));
    ext2_bg_desc_t result;
    read_fs((fs_t *)fs, &result, offset, sizeof(ext2_bg_desc_t));
    return result;
}

u64 get_inode_id_offset_in_fs(ext2_fs_t *fs, u32 inode_id) {
    ext2_bg_desc_t bg_desc = get_bg_desc(fs, INODE_BG(fs, inode_id));

    u32 index = (inode_id - 1) % fs->fs_info->inodes_per_group;

    u64 inode_table = bg_desc.inode_table;

    u32 inode_size;

    if (fs->fs_info->rev_level < 1)
        inode_size = 128;
    else
        inode_size = fs->fs_info->inode_size;

    return inode_table * fs->fs_info->block_size + (index * inode_size);
}

// read from disk the inode data from inode_id in the inode struct
// return a non-null value if an error occured
int read_inode(ext2_fs_t *fs, u32 inode_id, ext2_inode_t *inode) {
    u64 offset = get_inode_id_offset_in_fs(fs, inode_id);
    if (!offset) return 1;

    if (read_fs((fs_t *)fs, inode, offset, sizeof(ext2_inode_t))
        != sizeof(ext2_inode_t))
        return 1;

    return 0;
}

// write to disk inode data contained in inode for the inode of id inode_id
// return a non-null value if an error occured
int write_inode(ext2_fs_t *fs, u32 inode_id, ext2_inode_t *inode) {
    u64 offset = get_inode_id_offset_in_fs(fs, inode_id);
    if (!offset) return 1;

    if (write_fs((fs_t *)fs, inode, offset, sizeof(ext2_inode_t))
        != sizeof(ext2_inode_t))
        return 1;

    return 0;
}

// Return the index of the first 0 bit in the bitmap, the bitmap must span one
// full block of the fs and -1 is return is bitmap is full
i32 first_null_bit(ext2_fs_t *fs, u64 *bitmap) {
    u32 index = 0;
    while (bitmap[index++] == (u64)(-1)) {
        // bitmap has been fully scanned and no block is free
        if (index == fs->fs_info->block_size / 64) return -1;
    }

    u32 subindex = 0;
    while (bitmap[index] & (1 << subindex)) {
        subindex++;
    }

    return (index * 64) + subindex;
}

static inline void set_bit(u8 *bitmap, u32 index) {
    bitmap[index / 8] |= (u8)(1 << (index % 8));
}

static inline void clear_bit(u8 *bitmap, u32 index) {
    bitmap[index / 8] &= ~(u8)(1 << (index % 8));
}

// Allocate a new block for an inode
// Return 0 if it fails
u32 alloc_block(ext2_fs_t *fs, u32 inode_id) {
    ext2_inode_t inode;
    if (!read_inode(fs, inode_id, &inode)) return 0;

    u32 bg_id = INODE_BG(fs, inode_id);
    ext2_bg_desc_t bg_desc = get_bg_desc(fs, bg_id);
    u64 *bitmap = (u64 *)alloc(fs->fs_info->block_size);
    if (!read_fs((fs_t *)fs, bitmap, BLOCK_OFFSET(fs, bg_desc.block_bitmap),
                 fs->fs_info->block_size))
        return 0;

    i32 block = first_null_bit(fs, bitmap);

    if (block < -1) {
        bg_id = 0;
    }

    set_bit((u8 *)bitmap, block);
    // TODO not completed, need a better / working implmentation of IO to fisish
    // implementation

    return bg_id * fs->fs_info->blocks_per_group + (u32)block;
}

// Return a inode size taking into account
// the potential high 32 bit for a regular file
u64 get_inode_size(ext2_inode_t *inode) {
    u64 size = inode->size;

    // High 32 bit are in dir_acl for a regular file
    // It work even if it is a rev 0 ext2 fs because
    // in rev 0 dir_acl is always 0
    if ((inode->mode & 0xF000) == EXT2_IFREG) {
        size |= ((u64)inode->dir_acl << 32);
    }

    return size;
}

// return block_id at index in indirection pointers block
// level can either be 1, 2 or 3
u32 read_indirection(ext2_fs_t *fs, u32 block, u32 index, u8 level) {
    if (block == 0) return 0;
    // block per level, in log2
    u32 bpl = 10 + fs->fs_info->log_block_size - 2;

    u32 ind_index = index;
    while (level != 1) {
        u32 ind_block = ind_index / (bpl * level);
        ind_index %= (bpl * level);
        read_fs((fs_t *)fs, &block,
                block * fs->fs_info->block_size + 4 * ind_block, 4);
        if (block == 0) return 0;
        level--;
    }

    read_fs((fs_t *)fs, &block, block * fs->fs_info->block_size + 4 * ind_index,
            4);

    return block;
}

// Return block id of the inode block of index block_index
// Handle both direct pointers cases and indirect ones
// Return 0 if index is not allocated for the given inode
u32 get_inode_block(ext2_fs_t *fs, ext2_inode_t *inode, u32 block_index) {
    // Direct pointers
    if (block_index < 12) {
        return inode->block[block_index];
    }

    // Indirect pointers
    u32 bpl = 10 + fs->fs_info->log_block_size - 2;
    block_index -= 12;
    for (u8 ind_level = 1; ind_level <= 3; ind_level++) {
        if (block_index < (1 << (bpl * ind_level))) {
            return read_indirection(fs, inode->block[11 + ind_level],
                                    block_index, ind_level);
        }
        block_index -= (1 << (bpl * ind_level));
    }

    return 0;
}

// Read the content of inode into buffer, with a max size of len and offset into
// the inode
u64 read_from_inode(ext2_fs_t *fs, char *buffer, u64 len, u64 offset,
                    u32 inode_id) {
    ext2_inode_t inode;
    if (read_inode(fs, inode_id, &inode)) return 0;

    u64 inode_size = get_inode_size(&inode);

    if (offset + len > inode_size) len = inode_size - offset;

    u64 read = 0;
    u64 act_iter_read, max_iter_read;
    u32 bindex = offset / fs->fs_info->block_size;
    u32 bid;
    while (bindex * fs->fs_info->block_size < inode_size && read < len) {
        if (len + offset - read > fs->fs_info->block_size)
            max_iter_read = fs->fs_info->block_size;
        else
            max_iter_read = len + offset - read;

        bid = get_inode_block(fs, &inode, bindex);
        // we have a file hole
        if (bid == 0) {
            memset(buffer + read, 0, max_iter_read);
            read += max_iter_read;
            continue;
        }

        act_iter_read =
            read_fs((fs_t *)fs, buffer + read,
                    bid * fs->fs_info->block_size + offset, max_iter_read);
        if (act_iter_read != max_iter_read) {
            // error occured
            return read + act_iter_read;
        }

        offset = 0;
        read += max_iter_read;
        bindex++;
    }
    return read;
}

// write the first len bytes of buffer, at offset in content of the given inode
// return the number of bytes written, 0 if it fails
int write_to_inode(ext2_fs_t *fs, const char *buffer, u64 len, u64 offset,
                   u32 inode_id) {
    ext2_inode_t inode;
    if (read_inode(fs, inode_id, &inode)) return 0;

    u64 file_position = offset;
    u32 block_index = file_position / fs->fs_info->block_size;
    u32 block_id = get_inode_block(fs, &inode, block_index);
    if (block_id == 0) {
        // block index is not allocated, may happen if a file has hole
        // TODO not implemented
        return 0;
    }
    u32 block_offset = file_position % fs->fs_info->block_size;

    if (len > fs->fs_info->block_size - block_offset) {
        // overflow the current block
        return 0;
    }

    inode.size = offset + len;
    write_inode(fs, inode_id, &inode);

    return write_fs((fs_t *)fs, buffer, fs->fs_info->block_size * block_id,
                    len);
}

typedef struct path {
    char filename[255];
    char *subpath;
} path_t;

#define PATH_SEP '/'
int parse_path(char *path, path_t *parsed) {
    if (*(path++) != PATH_SEP) {
        // Path not valid
        return 1;
    }

    memset(parsed->filename, 0, 255);
    char *fname_start = path;
    while (*(path++) != '\0') {
        if (*path == PATH_SEP) {
            strncpy(parsed->filename, fname_start, path - fname_start);
            parsed->subpath = path;
            return 0;
        }
    }

    strncpy(parsed->filename, fname_start, path - fname_start);
    parsed->subpath = NULL;
    return 0;
}

// Return, if it exist, the inode id of the filename given in the dir
// Return 0 if it does not exists
u32 get_inode_in_dir(ext2_fs_t *fs, u32 dir, char *filename) {
    u16 path_len = strnlen(filename, 255);
    ext2_inode_t inode;
    read_inode(fs, dir, &inode);

    if (!(inode.mode & EXT2_IFDIR)) {
        return 0;
    }

    u32 dir_size = 512 * inode.blocks;
    u64 read = 0;
    if (inode.block[0]) {
        u32 reading_block = inode.block[0];
        u64 offset = reading_block * fs->fs_info->block_size;
        ext2_dir_entry_t entry;
        for (;;) {
            read_fs((fs_t *)fs, &entry, offset, sizeof(ext2_dir_entry_t));
            if (entry.name_len == path_len) {
                read_fs((fs_t *)fs, &entry, offset,
                        sizeof(ext2_dir_entry_t) + entry.name_len);
                if (strncmp(filename, (char *)entry.name, path_len) == 0) {
                    return entry.inode;
                }
            }
            // We have non-continguous data block for the directory
            // Only increment read to account for the allocated block
            if (entry.rec_len > fs->fs_info->block_size)
                read += entry.rec_len % fs->fs_info->block_size;
            else
                read += entry.rec_len;

            offset += entry.rec_len;

            if (read >= dir_size) break;
        }
    }
    return 0;
}

int list_dir(ext2_fs_t *fs, char *path) {
    path_t parsed;
    parse_path(path, &parsed);

    u32 dir_inum = EXT2_ROOT_DIR;
    while (*parsed.filename != '\0') {
        dir_inum = get_inode_in_dir(fs, dir_inum, parsed.filename);
        if (dir_inum == 0) return 1;
        if (!parsed.subpath) break;
        parse_path(parsed.subpath, &parsed);
    }

    ext2_inode_t inode;
    read_inode(fs, dir_inum, &inode);

    u64 offset = inode.block[0] * fs->fs_info->block_size;
    u64 dir_size = inode.blocks * 512;
    u64 read = 0;
    ext2_dir_entry_t dir;
    if (offset) {
        for (;;) {
            read_fs((fs_t *)fs, &dir, offset, sizeof(ext2_dir_entry_t));
            read_fs((fs_t *)fs, &dir, offset,
                    sizeof(ext2_dir_entry_t) + dir.name_len);
            printf("%.*s\n", dir.name_len, dir.name);
            // Non continguous data block
            if (dir.rec_len > fs->fs_info->block_size)
                read += dir.rec_len % fs->fs_info->block_size;
            else
                read += dir.rec_len;
            offset += dir.rec_len;

            if (read >= dir_size) break;
        }
    }

    return 0;
}

// Return the inode id of given file path
u32 get_inode_from_path(ext2_fs_t *fs, char *path) {
    path_t parsed;
    parse_path(path, &parsed);

    u32 dir_id = EXT2_ROOT_DIR;
    while (parsed.subpath) {
        // Path does not specify a file
        if (*parsed.filename == '\0') return 0;

        dir_id = get_inode_in_dir(fs, dir_id, parsed.filename);
        if (dir_id == 0) return 0;

        parse_path(parsed.subpath, &parsed);
    }

    return get_inode_in_dir(fs, dir_id, parsed.filename);
}

// Read at most length bytes from file specified by path
// Return the number of bytes read, 0 in case of an error
int read_from_file(ext2_fs_t *fs, char *path, void *buffer, u64 length) {
    u32 inode_id = get_inode_from_path(fs, path);
    // File does not exists
    if (!inode_id) return 0;

    return read_from_inode(fs, buffer, length, 0, inode_id);
}

int write_to_file(ext2_fs_t *fs, char *path, void *buffer, u64 len,
                  u64 offset) {
    u32 inode_id = get_inode_from_path(fs, path);

    // file does not exists
    if (!inode_id) return 0;

    return write_to_inode(fs, buffer, len, offset, inode_id);
}
