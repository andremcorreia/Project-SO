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

uint8_t const file_contents[] = "This is a string to write";

char buffer[1200];
int f;
int f2;

void *testing(){
    int g = tfs_copy_from_external_fs(path_src, target_path1);
    assert(g != -1);

    g = tfs_open(target_path1, 0);
    assert(g != -1);

    tfs_write(g,file_contents,sizeof(file_contents));

    tfs_read(g,buffer,sizeof(buffer));

    assert(tfs_close(g) != -1);

    return NULL;
}

void *testing2(){
    int g = tfs_copy_from_external_fs(path_src, target_path2);
    assert(g != -1);

    g = tfs_open(target_path2, 0);
    assert(g != -1);

    tfs_read(g,buffer,sizeof(buffer));

    tfs_write(g,file_contents,sizeof(file_contents));

    tfs_read(g,buffer,sizeof(buffer));

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
    }
    for (int i = 0; i < 1000; i+=3){
        pthread_join(tid[i], NULL);
        pthread_join(tid[i+1], NULL);
        pthread_join(tid[i+2], NULL);
    }

    printf("Successfully tested the threads\n");
    return 0;
}


