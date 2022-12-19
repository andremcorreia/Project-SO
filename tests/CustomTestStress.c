#include "fs/operations.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

 char *str_ext_file =
        "BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL Big BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL Big BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL Big BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL Big BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL Big BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL Big BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL BigL Big BigL BigL BigL BigL BigL Big";

char *path_src = "tests/CustomBig.txt";

char const target_path1[] = "/f1";
char const target_path2[] = "/f2";
char const link_path1[] = "/l1";
char const link_path2[] = "/l2";
char const link_path3[] = "/l3";
char const link_path4[] = "/l4";

char buffer[1200];
int f;
int f2;

void *testing(){
    int g = tfs_copy_from_external_fs(path_src, target_path1);
    assert(g != -1);

    g = tfs_open(target_path1, 0);
    assert(g != -1);

    assert(tfs_sym_link(target_path1, link_path1) != -1);
    assert(tfs_link(target_path1, link_path2) != -1);

    int h = tfs_open(link_path1, 0);
    assert(h != -1);
    int j = tfs_open(link_path2, 0);
    assert(j != -1);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path1) != -1);
    assert(tfs_unlink(link_path2) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}

int main() {
    pthread_t tid[3000];

    assert(tfs_init(NULL) != -1);

    f = tfs_open(target_path1, TFS_O_CREAT);
    assert(f != -1);
    assert(tfs_close(f) != -1);

    f2 = tfs_open(target_path1, TFS_O_CREAT);
    assert(f2 != -1);
    assert(tfs_close(f2) != -1);

    for (int i = 0; i < 1000; i+=3)
    {
        if (pthread_create(&tid[i], NULL, testing, (void*) NULL) != 0) {
            printf("Failed while testing the threads\n");
            return -1;
        }
    }

    for (int i = 0; i < 1000; i+=3)
    {
        pthread_join(tid[i], NULL);
    }
    printf("Successfully stress tested the threads\n");
    return 0;
}


