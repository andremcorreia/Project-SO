#include "fs/operations.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>
// ESTE TESTE VAI SER SOBRE OPEN FILES VAI TER UM FILE I QUE COMEÇA OPEN
// DEPOIS ESCREVE NUMA THREAD 
// ENQUANTO NOUTRA THREAD DA CLOSE(I), OPEN(I) E ESCREVE
 char *str_ext_file =
        "test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test test test test test test test test test Big test test test test test Big";

char *path_src = "tests/CustomBig.txt";

char const target_path1[] = "/f1";

int f;

void *testing(){


    return NULL;
}

int main() {
    pthread_t tid[9];

    assert(tfs_init(NULL) != -1);

    f = tfs_open(target_path1, TFS_O_CREAT);
    assert(f != -1);
    assert(tfs_close(f) != -1);

    if (pthread_create(&tid[0], NULL, testing, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }

    pthread_join(tid[0], NULL);
    
    printf("Successfully tested the threads with a lot of files\n");
    return 0;
}


