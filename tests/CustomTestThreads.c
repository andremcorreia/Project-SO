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

    //ssize_t r = tfs_read(g, buffer, sizeof(buffer) - 1);
    //assert(r == strlen(str_ext_file));
    //assert(!memcmp(buffer, str_ext_file, strlen(str_ext_file)));

    assert(tfs_sym_link(target_path1, link_path1) != -1);
    assert(tfs_link(target_path1, link_path2) != -1);

    int h = tfs_open(link_path1, 0);
    assert(h != -1);
    int j = tfs_open(link_path2, 0);
    assert(j != -1);
    //uint8_t buffery[sizeof(str_ext_file)];

    //assert(tfs_read(h, buffery, sizeof(buffery)) == sizeof(buffery));
    //assert(memcmp(buffery, str_ext_file, sizeof(buffery)) == 0);

    //uint8_t bufferu[sizeof(str_ext_file)];

    //assert(tfs_read(j, bufferu, sizeof(bufferu)) == sizeof(bufferu));
    //assert(memcmp(bufferu, str_ext_file, sizeof(bufferu)) == 0);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path1) != -1);
    assert(tfs_unlink(link_path2) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}

void *testing2(){
    int g = tfs_copy_from_external_fs(path_src, target_path2);
    assert(g != -1);

    g = tfs_open(target_path2, 0);
    assert(g != -1);

    //ssize_t r = tfs_read(g, buffer, sizeof(buffer) - 1);
    //assert(r == strlen(str_ext_file));
    //assert(!memcmp(buffer, str_ext_file, strlen(str_ext_file)));

    assert(tfs_sym_link(target_path2, link_path3) != -1);
    assert(tfs_link(target_path2, link_path4) != -1);

    int h = tfs_open(link_path3, 0);
    assert(h != -1);
    int j = tfs_open(link_path4, 0);
    assert(j != -1);
    //uint8_t bufferi[sizeof(str_ext_file)];

    //ssize_t a = tfs_read(h, bufferi, sizeof(bufferi));
    //printf("%ld___%ld\n", a, sizeof(bufferi));
    //assert(tfs_read(h, bufferi, sizeof(bufferi)) == sizeof(bufferi));
    //assert(memcmp(bufferi, str_ext_file, sizeof(bufferi)) == 0);

    //uint8_t bufferon[sizeof(str_ext_file)];

    //assert(tfs_read(j, bufferon, sizeof(bufferon)) == sizeof(bufferon));
    //assert(memcmp(bufferon, str_ext_file, sizeof(bufferon)) == 0);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path3) != -1);
    assert(tfs_unlink(link_path4) != -1);
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
        if (pthread_create(&tid[i+2], NULL, testing, (void*) NULL) != 0) {
            printf("Failed while testing the threads\n");
            return -1;
        }
        if (pthread_create(&tid[i+1], NULL, testing2, (void*) NULL) != 0) {
            printf("Failed while testing the threads\n");
            return -1;
        }

        pthread_join(tid[i], NULL);
        pthread_join(tid[i+1], NULL);
        pthread_join(tid[i+2], NULL);
    }
    printf("Successfully tested the threads\n");
    return 0;
}


