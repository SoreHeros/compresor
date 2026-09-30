//
// Created by heros on 31/01/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dictionary.h"
#include "difference.h"
#include "entropy.h"
#include "huffman.h"
#include "lists.h"
#include "sliding_window.h"
#include "entropy.h"

#define BUFFSIZ 0x10000

struct{
    char name[16];
    char extension[8];
    void (*compress)(char * source, char * dest);
    void (*decompress)(char * source, char * dest);
}algorithms[] = {
        {"dictionary", ".dict", dict_comp, dict_decomp},
        {"huffman", ".hfmn", huffman_comp, huffman_decomp},
        {"sliding window", ".slwd", sw_compress, sw_decompress},
        //{"difference", ".diff", difference_comp, difference_decomp}
    };

int are_equal(char * filename1, char * filename2){
    FILE * f1 = fopen(filename1, "rb"), * f2 = fopen(filename2, "rb");
    char buff1[BUFFSIZ], buff2[BUFFSIZ];

    if (f1 == NULL){
        printf("ERROR FILE %s COULDN'T BE OPENED\n", filename1);
        if (f2 != NULL)
            fclose(f2);
        return 0;
    }

    if (f2 == NULL){
        printf("ERROR FILE %s COULDN'T BE OPENED\n", filename2);
        if (f1 != NULL)
            fclose(f1);
        return 0;
    }

    size_t read1, read2;
    do{
        read1 = fread(buff1, 1, BUFFSIZ, f1);
        read2 = fread(buff2, 1, BUFFSIZ, f2);
        for (size_t i = 0; i < read1 && i < read2; i++){
            if (buff1[i] != buff2[i]){
                //files are not equal
                printf("ERROR AT CHARACTER %li f1:%c(%i) f2:%c(%i)\n", ftell(f1) - BUFFSIZ + i, buff1[i], buff1[i], buff2[i], buff2[i]);
                fclose(f1);
                fclose(f2);
                return 0;
            }
        }
    }while (read1 == BUFFSIZ && read2 == BUFFSIZ);

    if (read1 != read2){
        //files are not equal
        printf("ERROR FILES HAVE DIFFERENT LENGHTS\n");
        fclose(f1);
        fclose(f2);
        return 0;
    }

    fclose(f1);
    fclose(f2);
    return 1;
}

void do_simple(char * fileName, int algIndx){
    if (algIndx < 0 || (unsigned long)algIndx >= sizeof(algorithms) / sizeof(algorithms[0]))
        return;
    char * compFileName = strcat(realloc(strdup(fileName), strlen(fileName) + strlen(algorithms[algIndx].extension) + 1), algorithms[algIndx].extension);
    //FILE * in = fopen(fileName, "rb");
    //FILE * out = fopen(compFileName, "wb");

    printf("Compressing %s...\n", fileName);
    clock_t t1 = clock();
    algorithms[algIndx].compress(fileName, compFileName);
    clock_t t2 = clock();
    double deltaT = (t2 - t1) / (double)CLOCKS_PER_SEC;
    printf("Compressed in %lf s\n", deltaT);


    FILE * in = fopen(fileName, "rb");
    FILE * out = fopen(compFileName, "rb");

    fseek(in, 0, SEEK_END);
    fseek(out, 0, SEEK_END);

    long input_size = ftell(in), output_size = ftell(out);

    fclose(in);
    fclose(out);

    printf("%14s:\n", algorithms[algIndx].name);
    printf(" Original size: %10li Bytes\n", input_size);
    printf("Resulting size: %10li Bytes\n", output_size);
    printf("Resulting comp: %10.3lf%%\n", 100.0 - output_size*100.0/input_size);
    printf("   Total speed: %10.0lf kB/s\n", input_size / deltaT / 1024);
    printf("    Diff speed: %10.0lf kB/s\n", (input_size - output_size) / deltaT / 1024);

    printf("Decompressing %s...\n", compFileName);
    t1 = clock();
    algorithms[algIndx].decompress(compFileName, "decompressed.tmp");
    t2 = clock();
    deltaT = (t2 - t1) / (double)CLOCKS_PER_SEC;
    printf("Decompressed in %lf\n", deltaT);
    printf("   Total speed: %10.0lf kB/s\n", input_size / deltaT / 1024);

    if(are_equal(fileName, "decompressed.tmp"))
        printf("files are equal\n");
    else
        printf("files are NOT equal\n");

    printf("  Original entropy: %lf\n", measure_entropy(fileName));
    printf("Compressed entropy: %lf\n\n", measure_entropy(compFileName));

    free(compFileName);
}

typedef struct fileStruct{
    char fileName[256];
    size_t size;
    double compTitme;
} * fileStruct;

int file_comparator(const void * a, const void * b){
    if (((fileStruct)a)->size < ((fileStruct)b)->size)
        return 1;
    else if (((fileStruct)a)->size > ((fileStruct)b)->size)
        return -1;
    else
        return 0;
}

void do_all(char * fileName){
    list fileList = list_init();
    fileStruct ofs = malloc(sizeof(struct fileStruct));
    fileStruct best = ofs;
    int best_iter = 0;
    strcpy(ofs->fileName, fileName);
    ofs->compTitme = 0;
    FILE * f = fopen(fileName, "r");
    fseek(f, 0, SEEK_END);
    ofs->size = ftell(f);
    fclose(f);
    printf(" i|   Size   |   Comp   |   Time   |   Speed  |  (Diff)  |   Name\n");
    printf(" 0|%10li|%10.3lf|%10lf|%10.0lf|%10.0lf|   %s\n", ofs->size, 0.0, 0.0, 0.0, 0.0, ofs->fileName);
    list_ordered_insert(fileList, ofs, file_comparator);
    for (int i = 0; list_length(fileList); i++){
        fileStruct currfs = list_pop(fileList);
        for (int algIndx = 0; (unsigned long)algIndx < sizeof(algorithms) / sizeof(algorithms[0]); algIndx++){
            fileStruct tempfs = malloc(sizeof(struct fileStruct));
            strcpy(tempfs->fileName, currfs->fileName);
            strcat(tempfs->fileName, algorithms[algIndx].extension);
            clock_t start = clock();
            algorithms[algIndx].compress(currfs->fileName, tempfs->fileName);
            clock_t end = clock();
            tempfs->compTitme = currfs->compTitme + (end - start) / (double)CLOCKS_PER_SEC;
            f = fopen(tempfs->fileName, "rb");
            fseek(f, 0, SEEK_END);
            tempfs->size = ftell(f);
            fclose(f);

            if(tempfs->size < best->size){
                if (best != currfs && best != ofs && !list_exists(fileList, best)){
                    remove(best->fileName);
                    free(best);
                }
                best = tempfs;
                best_iter = i;
            }

            printf("%2i|%10li|%10.3lf|%10lf|%10.0lf|%10.0lf|   %s\n",i, tempfs->size, 100.0 - tempfs->size*100.0/ofs->size, tempfs->compTitme, ofs->size / tempfs->compTitme / 1024, (ofs->size - (double)tempfs->size) / tempfs->compTitme / 1024, tempfs->fileName);

            if (tempfs->size < currfs->size)
                list_ordered_insert(fileList, tempfs, file_comparator);
            else{
                remove(tempfs->fileName);
                free(tempfs);
            }
        }
        if (currfs != best && currfs != ofs){
            remove(currfs->fileName);
            free(currfs);
        }
    }

    printf("\nBest was: %s Size:%li Comp:%lf%% Iter:%i\n", best->fileName, best->size, 100.0 - best->size*100.0/ofs->size, best_iter);

    if (best != ofs)
        free(best);
    free(ofs);
    while(list_length(fileList))
        free(list_pop(fileList));
    list_free(fileList);
}

//todo file validation
int main(int len, char ** arr){
    clock_t start = clock();
    char * fileName = "main.c";
    int alg = -1;
    int fileIndx = 2;



    if (len > 1){
        if(!strcmp(arr[1], "-all"))
            alg = -1;
        else if(arr[1][0] == '-')
            alg = atoi(&arr[1][1]);
        else
            fileIndx = 1;
    }

    do{
        if(len > fileIndx)
            fileName = arr[fileIndx];

        if(alg == -1)
            do_all(fileName);
        else
            do_simple(fileName, alg);

        fileIndx++;
    }while (len > fileIndx);

    clock_t end = clock();

    printf("Total time: %lf\n", (end - start) / (double)CLOCKS_PER_SEC);

    return 0;
}