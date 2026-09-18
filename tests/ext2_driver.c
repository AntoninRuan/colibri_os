#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int16_t i16;
#include <kernel/driver/ext2.h>

int main(void) {
    FILE *f = fopen("../disk.img", "r+");
    if (!f) return 1;

    ext2_fs_t fs;
    mount(f, &fs);

    char *path = "/test_dir/file_in_dir";

    char *buffer = malloc(255);
    memset(buffer, 0, 255);
    // buffer = "foo bar boo";
    // u32 len = write_to_file(&fs, path, buffer, strlen(buffer), 0);
    u32 len = read_from_file(&fs, path, buffer, 255);
    if (!len) {
        printf("Error while reading file\n");
        return 1;
    }

    printf("%s content is:\n", path);
    printf("%.*s\n", len, buffer);

    // umount(&fs);

    return fclose(f);
}
