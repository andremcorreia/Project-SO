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
char const target_path3[] = "/f3";
char const target_path4[] = "/f4";
char const target_path5[] = "/f5";
char const target_path6[] = "/f6";
char const target_path7[] = "/f7";
char const target_path8[] = "/f8";
char const target_path9[] = "/f9";
char const link_path1[] = "/l1";
char const link_path2[] = "/l2";
char const link_path3[] = "/l3";
char const link_path4[] = "/l4";
char const link_path5[] = "/l5";
char const link_path6[] = "/l6";
char const link_path7[] = "/l7";
char const link_path8[] = "/l8";
char const link_path9[] = "/l9";
char const link_path10[] = "/l10";
char const link_path11[] = "/l11";
char const link_path12[] = "/l12";
char const link_path13[] = "/l13";
char const link_path14[] = "/l14";
char const link_path15[] = "/l15";
char const link_path16[] = "/l16";
char const link_path17[] = "/l17";
char const link_path18[] = "/l18";

char buffer[1200];
int f;
int f2;
int f3;
int f4;
int f5;
int f6;
int f7;
int f8;
int f9;

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

void *testing2(){
    int g = tfs_copy_from_external_fs(path_src, target_path2);
    assert(g != -1);

    g = tfs_open(target_path2, 0);
    assert(g != -1);

    assert(tfs_sym_link(target_path2, link_path3) != -1);
    assert(tfs_link(target_path2, link_path4) != -1);

    int h = tfs_open(link_path3, 0);
    assert(h != -1);
    int j = tfs_open(link_path4, 0);
    assert(j != -1);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path3) != -1);
    assert(tfs_unlink(link_path4) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}

void *testing3(){
    int g = tfs_copy_from_external_fs(path_src, target_path3);
    assert(g != -1);

    g = tfs_open(target_path3, 0);
    assert(g != -1);

    assert(tfs_sym_link(target_path3, link_path5) != -1);
    assert(tfs_link(target_path3, link_path6) != -1);

    int h = tfs_open(link_path5, 0);
    assert(h != -1);
    int j = tfs_open(link_path6, 0);
    assert(j != -1);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path5) != -1);
    assert(tfs_unlink(link_path6) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}

void *testing4(){
    int g = tfs_copy_from_external_fs(path_src, target_path4);
    assert(g != -1);

    g = tfs_open(target_path4, 0);
    assert(g != -1);

    assert(tfs_sym_link(target_path4, link_path7) != -1);
    assert(tfs_link(target_path4, link_path8) != -1);

    int h = tfs_open(link_path7, 0);
    assert(h != -1);
    int j = tfs_open(link_path8, 0);
    assert(j != -1);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path7) != -1);
    assert(tfs_unlink(link_path8) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}

void *testing5(){
    int g = tfs_copy_from_external_fs(path_src, target_path5);
    assert(g != -1);

    g = tfs_open(target_path5, 0);
    assert(g != -1);

    assert(tfs_sym_link(target_path5, link_path9) != -1);
    assert(tfs_link(target_path5, link_path10) != -1);

    int h = tfs_open(link_path9, 0);
    assert(h != -1);
    int j = tfs_open(link_path10, 0);
    assert(j != -1);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path9) != -1);
    assert(tfs_unlink(link_path10) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}

void *testing6(){
    int g = tfs_copy_from_external_fs(path_src, target_path6);
    assert(g != -1);

    g = tfs_open(target_path6, 0);
    assert(g != -1);

    assert(tfs_sym_link(target_path6, link_path11) != -1);
    assert(tfs_link(target_path6, link_path12) != -1);

    int h = tfs_open(link_path11, 0);
    assert(h != -1);
    int j = tfs_open(link_path12, 0);
    assert(j != -1);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path11) != -1);
    assert(tfs_unlink(link_path12) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}

void *testing7(){
    int g = tfs_copy_from_external_fs(path_src, target_path7);
    assert(g != -1);

    g = tfs_open(target_path7, 0);
    assert(g != -1);

    assert(tfs_sym_link(target_path7, link_path13) != -1);
    assert(tfs_link(target_path7, link_path14) != -1);

    int h = tfs_open(link_path13, 0);
    assert(h != -1);
    int j = tfs_open(link_path14, 0);
    assert(j != -1);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path13) != -1);
    assert(tfs_unlink(link_path14) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}

void *testing8(){
    int g = tfs_copy_from_external_fs(path_src, target_path8);
    assert(g != -1);

    g = tfs_open(target_path8, 0);
    assert(g != -1);

    assert(tfs_sym_link(target_path8, link_path15) != -1);
    assert(tfs_link(target_path8, link_path16) != -1);

    int h = tfs_open(link_path15, 0);
    assert(h != -1);
    int j = tfs_open(link_path16, 0);
    assert(j != -1);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path15) != -1);
    assert(tfs_unlink(link_path16) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}

void *testing9(){
    int g = tfs_copy_from_external_fs(path_src, target_path9);
    assert(g != -1);

    g = tfs_open(target_path9, 0);
    assert(g != -1);

    assert(tfs_sym_link(target_path9, link_path17) != -1);
    assert(tfs_link(target_path9, link_path18) != -1);

    int h = tfs_open(link_path17, 0);
    assert(h != -1);
    int j = tfs_open(link_path18, 0);
    assert(j != -1);

    assert(tfs_close(h) != -1);
    assert(tfs_close(j) != -1);
    assert(tfs_unlink(link_path17) != -1);
    assert(tfs_unlink(link_path18) != -1);
    assert(tfs_close(g) != -1);

    return NULL;
}


int main() {
    pthread_t tid[9];

    assert(tfs_init(NULL) != -1);

    f = tfs_open(target_path1, TFS_O_CREAT);
    assert(f != -1);
    assert(tfs_close(f) != -1);

    f2 = tfs_open(target_path1, TFS_O_CREAT);
    assert(f2 != -1);
    assert(tfs_close(f2) != -1);

    f3 = tfs_open(target_path1, TFS_O_CREAT);
    assert(f3 != -1);
    assert(tfs_close(f3) != -1);

    f4 = tfs_open(target_path1, TFS_O_CREAT);
    assert(f4 != -1);
    assert(tfs_close(f4) != -1);

    f5 = tfs_open(target_path1, TFS_O_CREAT);
    assert(f5 != -1);
    assert(tfs_close(f5) != -1);

    f6 = tfs_open(target_path1, TFS_O_CREAT);
    assert(f6 != -1);
    assert(tfs_close(f6) != -1);

    f7 = tfs_open(target_path1, TFS_O_CREAT);
    assert(f7 != -1);
    assert(tfs_close(f7) != -1);

    f8 = tfs_open(target_path1, TFS_O_CREAT);
    assert(f8 != -1);
    assert(tfs_close(f8) != -1);

    f9 = tfs_open(target_path1, TFS_O_CREAT);
    assert(f9 != -1);
    assert(tfs_close(f9) != -1);

    if (pthread_create(&tid[0], NULL, testing, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    if (pthread_create(&tid[1], NULL, testing, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    if (pthread_create(&tid[2], NULL, testing2, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    if (pthread_create(&tid[3], NULL, testing2, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    if (pthread_create(&tid[4], NULL, testing, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    if (pthread_create(&tid[5], NULL, testing, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    if (pthread_create(&tid[6], NULL, testing, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    if (pthread_create(&tid[7], NULL, testing, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }
    if (pthread_create(&tid[8], NULL, testing2, (void*) NULL) != 0) {
        printf("Failed while testing the threads\n");
        return -1;
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);
    pthread_join(tid[3], NULL);
    pthread_join(tid[4], NULL);
    pthread_join(tid[5], NULL);
    pthread_join(tid[6], NULL);
    pthread_join(tid[7], NULL);
    pthread_join(tid[8], NULL);
    
    printf("Successfully tested the threads with a lot of files\n");
    return 0;
}


