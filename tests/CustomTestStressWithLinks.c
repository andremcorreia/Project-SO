#include "fs/operations.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

 char *str_ext_file =
        "test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test Big";

char *path_src = "tests/CustomBig.txt";

char const target_path1[] = "/f1";
char const link_path1[] = "/l";

char buffer[1200];
int f;

void *testing(void* arg){
    long unsigned link_num = (uintptr_t)arg;

    int g = tfs_copy_from_external_fs(path_src, target_path1);
    assert(g != -1);

    g = tfs_open(target_path1, 0);
    assert(g != -1);

    char sym_l[128];
    snprintf(sym_l, 128, "%s%ld", link_path1, link_num);
    char hard_l[128];
    snprintf(hard_l, 128, "%s%ld", link_path1, link_num+1);
    assert(tfs_sym_link(target_path1, sym_l) != -1);
    assert(tfs_link(target_path1, hard_l) != -1);

    int h = tfs_open(sym_l, 0);
    assert(h != -1);
    int j = tfs_open(hard_l, 0);
    assert(j != -1);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(sym_l) != -1);
    assert(tfs_unlink(hard_l) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}

int main() {
    pthread_t tid[2000];

    assert(tfs_init(NULL) != -1);

    f = tfs_open(target_path1, TFS_O_CREAT);
    assert(f != -1);
    assert(tfs_close(f) != -1);

    for (int i = 0; i <= 1000; i+=2)
    {
        if (pthread_create(&tid[i], NULL, (void*)testing, (void*) (uintptr_t)i) != 0) {
            printf("Failed while testing the threads\n");
            return -1;
        }
    }
    for (int i = 0; i <= 1000; i+=2)
    {
        pthread_join(tid[i], NULL);
    }
    printf("Successfully stress tested the threads with links\n");
    return 0;
}


