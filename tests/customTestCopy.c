#include "fs/operations.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    char *str_ext_file =
        "test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test Big";
    char *str_ext_too_big_file =
        "test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test";
    char * str_ovewrite_tests = "hello, this is a test file, f1 is not among us.";

    char *path_copied_file = "/f1";
    char *path_copied_too_big_file = "/f2";
    char *path_but_too_big = "/NeverGonnaGiveYouUpNeverGonnaLetYouDownNeverGonnaRunAroundAndHurtYou.";
    
    char *path_src = "tests/CustomBig.txt";
    char *path_src2 = "tests/CustomForSym.txt";
    char *path_error_src = "tests/CustomTooBig.txt";

    char const link_path1[] = "/l1";

    char buffer[1200];

    assert(tfs_init(NULL) != -1);

    int f;
    ssize_t r;

    // Tests a path name over limit
    assert(tfs_copy_from_external_fs(path_src, path_but_too_big) == -1);

    f = tfs_copy_from_external_fs(path_src, path_copied_file);
    assert(f != -1);

    f = tfs_open(path_copied_file, TFS_O_CREAT);
    assert(f != -1);

    r = tfs_read(f, buffer, sizeof(buffer) - 1);
    assert(r == strlen(str_ext_file));
    assert(!memcmp(buffer, str_ext_file, strlen(str_ext_file)));

    printf("Successfully tested: |Path name over limit|.\n");

    int f2;
    ssize_t r2;

    // Tests a file over size limit
    f2 = tfs_copy_from_external_fs(path_error_src, path_copied_too_big_file);
    assert(f2 != -1);

    f2 = tfs_open(path_copied_too_big_file, TFS_O_CREAT);
    assert(f2 != -1);

    r2 = tfs_read(f2, buffer, sizeof(buffer) - 1);
    assert(r2 == strlen(str_ext_too_big_file) - 11); //tests if it stopped copying at the limit
    assert(memcmp(buffer, str_ext_too_big_file, strlen(str_ext_too_big_file)));

    printf("Successfully tested: |File over size limit|.\n");

    int f3;
    ssize_t r3;

    assert(tfs_sym_link(path_copied_file, link_path1) != -1); //create a sym link
    f3 = tfs_open(link_path1, 0);
    assert(f3 != -1);

    u_int8_t buffer2[sizeof(link_path1)];
    assert(tfs_read(f3, buffer2, sizeof(buffer2)) > 0);

    assert(tfs_close(f3) != -1);

    f3 = tfs_copy_from_external_fs(path_src2, link_path1); //overwrite a sym link with a system file
    assert(f3 != -1);
    f3 = tfs_open(link_path1, 0); 
    assert(f3 != -1);

    r3 = tfs_read(f3, buffer, sizeof(buffer) - 1);
    assert(r != r3); // checks if its replacing the sym link with a normal file

    printf("Successfully tested: |Copied into a symbolic link|.\n");

    int f4;
    ssize_t r4;

    // Tests the truncation of an existing file for it to be copied
    f4 = tfs_copy_from_external_fs(path_src, path_copied_file);
    assert(f4 != -1);

    f4 = tfs_copy_from_external_fs(path_src2, path_copied_file);
    assert(f4 != -1);

    f4 = tfs_open(path_copied_file, TFS_O_CREAT);
    assert(f4 != -1);

    r4 = tfs_read(f4, buffer, sizeof(buffer) - 1);
    assert(r4 == strlen(str_ovewrite_tests));
    assert(!memcmp(buffer, str_ovewrite_tests, strlen(str_ovewrite_tests)));

    printf("Successfully tested: |Truncated an existing file in order to copy|.\n");
}
