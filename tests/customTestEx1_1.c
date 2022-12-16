#include "../fs/operations.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    char *str_ext_file =
        "BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL "
        "BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL "
        "BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL "
        "BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL "
        "BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL "
        "BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL "
        "BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL "
        "BigL BigL BigL BigL BigL ";

    char *path_copied_file = "/f1";
    char *path_but_too_big = "/NeverGonnaGiveYouUpNeverGonnaLetYouDownNeverGonnaRunAroundAndHurtYou.";
    char *path_src = "tests/CustomBig.txt";
    char buffer[600];

    assert(tfs_init(NULL) != -1);

    int f;
    ssize_t r;

    assert(tfs_copy_from_external_fs(path_src, path_but_too_big) == -1);

    f = tfs_copy_from_external_fs(path_src, path_copied_file);
    assert(f != -1);

    f = tfs_open(path_copied_file, TFS_O_CREAT);
    assert(f != -1);

    r = tfs_read(f, buffer, sizeof(buffer) - 1);
    assert(r == strlen(str_ext_file));
    assert(!memcmp(buffer, str_ext_file, strlen(str_ext_file)));

    printf("Successful Custom test.\n");
}
