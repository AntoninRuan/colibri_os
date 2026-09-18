#ifndef EXT2_H
#define EXT2_H

#include <stdio.h>
#include <sys/cdefs.h>

typedef enum {
    EXT2,
    FAT,
} FS_TYPE;

typedef struct {
    FS_TYPE type;
    FILE* fs_img;
    void* fs_info;
} fs_t;

// Memory representation
typedef struct {
    // Disk superblock fields
    u32 inodes_count;
    u32 blocks_count;
    u32 r_blocks_count;
    u32 free_blocks_count;
    u32 free_inodes_count;
    u32 first_data_block;
    u32 log_block_size;    // real size = 1024 << log_block_size
    u32 log_frag_size;     // same comment
    u32 blocks_per_group;  // block group = bg
    u32 frags_per_group;
    u32 inodes_per_group;
    u32 mtime;          // POSIX time
    u32 wtime;          // POSIX time
    i16 mnt_count;      // Mount count since last check
    i16 max_mnt_count;  // Max mount count without check
    u16 magic;          // Should be 0xEF53
    u16 state;
    u16 errors;
    u16 minor_rev_level;
    u32 lastcheck;      // POSIX time, last time since check
    u32 checkinterval;  // POSIX time, max time wihtout check
    u32 creator_os;
    u32 rev_level;
    u16 def_resuid;
    u16 def_resgid;
    // Next fields are only valid if s_rev_level >= 1
    u32 first_ino;
    u16 inode_size;
    u16 block_group_nr;
    u32 feature_compat;
    u32 feature_incompat;
    u32 feature_ro_compat;  // if one is missing fs need to be mounted RO
    u8 uuid[16];
    u8 volume_name[16];   // C string
    u8 last_mounted[64];  // C string, path
    u32 algo_bitmap;
    u8 prealloc_blocks;
    u8 prealloc_dir_blocks;
    u16 alignment;
    u8 journal_uuid[16];
    u32 journal_inum;
    u32 journal_dev;
    u32 last_orphan;
    u32 hash_seed[4];
    u8 def_hash_version;
    u8 padding[3];
    u32 default_mount_options;
    u32 first_meta_bg;

    // Useful fs informations
    u8 readonly;
    u64 block_size;
} ext2_sb_info_t;

// Disk representation
typedef struct {
    u32 inodes_count;
    u32 blocks_count;
    u32 r_blocks_count;
    u32 free_blocks_count;
    u32 free_inodes_count;
    u32 first_data_block;
    u32 log_block_size;    // real size = 1024 << log_block_size
    u32 log_frag_size;     // same comment
    u32 blocks_per_group;  // block group = bg
    u32 frags_per_group;
    u32 inodes_per_group;
    u32 mtime;          // POSIX time
    u32 wtime;          // POSIX time
    i16 mnt_count;      // Mount count since last check
    i16 max_mnt_count;  // Max mount count without check
    u16 magic;          // Should be 0xEF53
    u16 state;
    u16 errors;
    u16 minor_rev_level;
    u32 lastcheck;      // POSIX time, last time since check
    u32 checkinterval;  // POSIX time, max time wihtout check
    u32 creator_os;
    u32 rev_level;
    u16 def_resuid;
    u16 def_resgid;
    // Next fields are only valid if s_rev_level >= 1
    u32 first_ino;
    u16 inode_size;
    u16 block_group_nr;
    u32 feature_compat;
    u32 feature_incompat;
    u32 feature_ro_compat;  // if one is missing fs need to be mounted RO
    u8 uuid[16];
    u8 volume_name[16];   // C string
    u8 last_mounted[64];  // C string, path
    u32 algo_bitmap;
    u8 prealloc_blocks;
    u8 prealloc_dir_blocks;
    u16 alignment;
    u8 journal_uuid[16];
    u32 journal_inum;
    u32 journal_dev;
    u32 last_orphan;
    u32 hash_seed[4];
    u8 def_hash_version;
    u8 padding[3];
    u32 default_mount_options;
    u32 first_meta_bg;
    u8 unused[760];
} __attribute__((packed)) ext2_superblock_t;

// magic
#define EXT2_MAGIC                          0xEF53
// state
#define EXT2_UNCLEAN_FS                     0
#define EXT2_VALID_FS                       1
#define EXT2_ERROR_FS                       2
// errors
#define EXT2_ERRORS_CONTINUE                1
#define EXT2_ERRORS_RO                      2
#define EXT2_ERRORS_PANIC                   3
// creator_os
#define EXT2_OS_LINUX                       0
#define EXT2_OS_HURD                        1
#define EXT2_OS_MASIX                       2
#define EXT2_OS_FREEBSD                     3
#define EXT2_OS_LITES                       4
// feature_compat
#define EXT2_FEATURE_COMPAT_DIR_PREALLOC    0x1
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES   0x2
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL     0x4
#define EXT2_FEATURE_COMPAT_EXT_ATTR        0x8
#define EXT2_FEATURE_COMPAT_RESIZE_INO      0x10
#define EXT2_FEATURE_COMPAT_DIR_INDEX       0x20
// feature_incompat
#define EXT2_FEATURE_INCOMPAT_COMPRESSION   0x1
#define EXT2_FEATURE_INCOMPAT_FILETYPE      0x2
#define EXT2_FEATURE_INCOMPAT_RECOVER       0x4
#define EXT2_FEATURE_INCOMPAT_JOURNAL_DEV   0x8
#define EXT2_FEATURE_INCOMPAT_META_BG       0x10
// feature_ro_compat
#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER 0x1
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE   0x2
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR    0x4
// algo bitmap
#define EXT2_LZV1_ALG                       0x1
#define EXT2_LZRW3A_ALG                     0x2
#define EXT2_GZIP_ALG                       0x4
#define EXT2_BZIP2_ALG                      0x8
#define EXT2_LZO_ALG                        0x10

typedef struct {
    FS_TYPE type;  // Must always be EXT2
    FILE* fs_img;
    ext2_sb_info_t* fs_info;
} ext2_fs_t;

typedef struct {
    u32 block_bitmap;  // block address of block usage bitmap
    u32 inode_bitmap;  // block address of inode usage bitmap
    u32 inode_table;   // starting block address of inode table
    u16 free_blocks_count;
    u16 free_inodes_count;
    u16 used_dirs_count;
    u16 padding;
    u8 reserved[12];
} ext2_bg_desc_t;

typedef struct {
    u16 mode;
    u16 uid;
    u32 size;
    u32 atime;  // POSIX time, last access
    u32 ctime;  // POSIX time, creation
    u32 mtime;  // POSIX time, modification
    u32 dtime;  // POSIX time, deletion
    u16 gid;
    u16 links_count;
    u32 blocks;  // number of 512-bytes (not ext2) blocks used
    u32 flags;
    u32 osd1;
    u32 block[15];  // First 12 are direct, block[12] single indirection,
                    // block[13] double indirection and block [14] triple
    u32 generation;
    u32 file_acl;  // Only for ext2 >= 1.0
    u32 dir_acl;   // Only for ext2 >= 1.0, contains upper 32 bit of size for
                   // files
    u32 faddr;
    u8 osd2[12];
} ext2_inode_t;

// mode
#define EXT2_IFSOCK 0xC000  // socket
#define EXT2_IFLNK  0xA000  // symbolic link
#define EXT2_IFREG  0x8000  // regular file
#define EXT2_IFBLK  0x6000  // lock device
#define EXT2_IFDIR  0x4000  // directory
#define EXT2_IFCHR  0x2000  // character device
#define EXT2_IFIFO  0x1000  // fifo

#define EXT2_ISUID 0x0800  // set process user id
#define EXT2_ISGID 0x0400  //  set process group id
#define EXT2_ISVTX 0x0200  // sticky bit

#define EXT2_S_IRUSR 0x0100  // user read
#define EXT2_S_IWUSR 0x0080  // user write
#define EXT2_S_IXUSR 0x0040  // user execute
#define EXT2_S_IRGRP 0x0020  // group read
#define EXT2_S_IWGRP 0x0010  // group write
#define EXT2_S_IXGRP 0x0008  // group execute
#define EXT2_S_IROTH 0x0004  // other read
#define EXT2_S_IWOTH 0x0002  // other write
#define EXT2_S_IXOTH 0x0001  // other execute

// flags
#define EXT2_SECRM_FL        0x1   // Secure deletion
#define EXT2_UNRM_FL         0x2   // record for undelete
#define EXT2_COMPR_FL        0x4   // compressed file
#define EXT2_SYNC_FL         0x8   // synchronous update
#define EXT2_IMMUTABLE_FL    0x10  // immutable file
#define EXT2_APPEND_FL       0x20  // append only
#define EXT2_NODUMP_FL       0x40  // do not dump/delete file
#define EXT2_NOATIME_FL      0x80  // do not update atime
// Flags used for compression usage
#define EXT2_DIRTY_FL        0x100  // dirty
#define EXT2_COMPRBLK_FL     0x200  // compressed blocks
#define EXT2_NOCOMPR_FL      0x400  // access raw compressed data
#define EXT2_ECOMPR_FL       0x800  // compression error
// End of compression flags
#define EXT2_BTREE_FL        0x1000  // b-tree format directory
#define EXT2_INDEX_FL        0x2000  // has indexed directory
#define EXT2_IMAGIC_FL       0x3000  // AFS directory
#define EXT2_JOURNAL_DATA_FL 0x4000  // Journal file data

typedef struct {
    u32 inode;
    u16 rec_len;
    u8 name_len;
    u8 file_type;
    u8 name[255];
} ext2_dir_entry_t;

// file type
#define EXT2_FT_UNKNOWN  0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR      2
#define EXT2_FT_CHRDEV   3
#define EXT2_FT_BLKDEV   4
#define EXT2_FT_FIFO     5  // buffer file
#define EXT2_FT_SOCK     6
#define EXT2_FT_SYMLINK  7

#define EXT2_ROOT_DIR 2

int mount(FILE* img, ext2_fs_t* fs);
int umount(ext2_fs_t* fs);
int list_dir(ext2_fs_t*, char* path);
int read_from_file(ext2_fs_t*, char* path, void* buffer, u64 length);
int write_to_file(ext2_fs_t*, char* path, void* buffer, u64 len, u64 offset);

#endif  // EXT2_H
