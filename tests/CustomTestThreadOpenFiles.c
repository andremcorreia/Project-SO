#include "fs/operations.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

char const target_path1[] = "/f1";

uint8_t const file_contents[] = "istecnico";

// Tests file opening and closing

int f;
void *testingWrite(){

    int g = tfs_open(target_path1, 0);
    assert(g != -1);
    for (int i = 0; i < 100; i++)
    {
        tfs_write(g,file_contents,sizeof(file_contents)); 
    }
    assert(tfs_close(g) != -1);

    return NULL;
}

void *testingOpenFiles(){

    for (int i = 0; i < 100; i++)
    {
        assert(tfs_close(f) != -1);

        f = tfs_open(target_path1, 0);
    
        tfs_write(f,file_contents,sizeof(file_contents));
    }

    return NULL;
}
 
int main() {
    pthread_t tid[2];

    assert(tfs_init(NULL) != -1);

    f = tfs_open(target_path1, TFS_O_CREAT);
    assert(f != -1);

    if (pthread_create(&tid[0], NULL, testingWrite, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
     if (pthread_create(&tid[1], NULL, testingOpenFiles, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    
    printf("Successfully tested openning, writing and closing with threads\n");
    return 0;
}


