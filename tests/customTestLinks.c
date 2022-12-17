#include "fs/operations.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t const file_contents[] = "There is text here I swear";
char const target_path1[] = "/f1";
char const link_path1[] = "/l1";
char const link_path2[] = "/l2";
char const link_path3[] = "/l3";
char const link_path4[] = "/l4";
char const link_path5[] = "/l5";

void assert_contents_ok(char const *path) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(file_contents)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == sizeof(buffer));
    assert(memcmp(buffer, file_contents, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);
}

void assert_empty_file(char const *path) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(file_contents)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);
}

void write_contents(char const *path) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    assert(tfs_write(f, file_contents, sizeof(file_contents)) ==
           sizeof(file_contents));

    assert(tfs_close(f) != -1);
}

int main() {

    // init TécnicoFS
    assert(tfs_init(NULL) != -1);

    assert(tfs_sym_link(target_path1, link_path1) != -1);
    assert(tfs_open(link_path1, TFS_O_APPEND) == -1);
    assert(tfs_unlink(link_path1) != -1);
    printf("Successfully tested: |symbolically linked to an unexisting file|.\n");

    assert(tfs_link(target_path1, link_path1) == -1);
    printf("Successfully tested: |linked to an unexisting file|.\n");

    // Create a file
    int f = tfs_open(target_path1, TFS_O_CREAT);
    assert(f != -1);
    assert(tfs_close(f) != -1);

    assert_empty_file(target_path1); // sanity check
    write_contents(target_path1);
    assert_contents_ok(target_path1);

    assert(tfs_sym_link(target_path1, link_path1) != -1);
    assert(tfs_sym_link(link_path1, link_path2) != -1);
    assert(tfs_sym_link(link_path2, link_path3) != -1);
    assert(tfs_sym_link(link_path3, link_path4) != -1);
    assert(tfs_sym_link(link_path4, link_path5) != -1);
    
    assert_contents_ok(link_path5);
    printf("Successfully tested: |symbolic link chain|.\n");
    return 0;
}
