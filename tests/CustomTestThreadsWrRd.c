#include "fs/operations.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

char *path_src = "tests/CustomBig.txt";

char const target_path1[] = "/f1";

uint8_t const file_contents[] = "istecnico";

int f;
bool end = true;
/* Test the creation of 3 simultaneous threads and do:
*
* 2 threads reading in a loop while 1 writes, when everything is written stop the reads.
*/
void *testingRead(){
    int g = tfs_copy_from_external_fs(path_src, target_path1);
    assert(g != -1);

    g = tfs_open(target_path1, 0);
    assert(g != -1);
    char buffer[2];
    while (1) {
        assert(tfs_read(g,buffer,sizeof(buffer)) % 2 != 0) //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
                                                // VER ISTO MELHOR TEM DE CONSEGUIR LER SEMPRE UM NUMERO DIVISIVEL POR 2
        if(!end)
            break; // quando acaba de escrever tudo acaba de ler
    }

    assert(tfs_close(g) != -1);

    return NULL;
}

void *testingWrite(){
    int g = tfs_copy_from_external_fs(path_src, target_path1);
    assert(g != -1);

    g = tfs_open(target_path1, 0);
    assert(g != -1);
    for (int i = 0; i < 100; i++)
    {
        tfs_write(g,file_contents,sizeof(file_contents)); // escreve tudo 
    }
    end = false;    // acabou de escrever
    assert(tfs_close(g) != -1);

    return NULL;
}

int main() {
    pthread_t tid[3];

    assert(tfs_init(NULL) != -1);

    f = tfs_open(target_path1, TFS_O_CREAT);
    assert(f != -1);
    assert(tfs_close(f) != -1);
     if (pthread_create(&tid[0], NULL, testingRead, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    if (pthread_create(&tid[2], NULL, testingWrite, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    if (pthread_create(&tid[1], NULL, testingRead, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);


    printf("Successfully tested the threads\n");
    return 0;
}


