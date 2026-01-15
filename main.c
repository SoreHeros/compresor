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

struct{
    char name[16];
    char extension[8];
    void (*compress)(FILE * source, FILE * dest);
    void (*decompress)(FILE * source, FILE * dest);
}algorithms[] = {
        {"dictionary", ".dict", dict_comp, dict_decomp},
        {"huffman", ".hfmn", huffman_comp, huffman_decomp},
        {"sliding window", ".slwd", sw_compress, sw_decompress},
        //{"difference", ".diff", difference_comp, difference_decomp}
    };

int are_equal(FILE * f1, FILE * f2){
    rewind(f1);
    rewind(f2);
    int c1, c2;
    do{
        c1 = fgetc(f1);
        c2 = fgetc(f2);
    }while(c1 == c2 && c1 != EOF && c2 != EOF);

    if(c1 == EOF && c2 == EOF)
        return 1;

    printf("ERROR AT CHARACTER %li f1:%c(%i) f2:%c(%i)\n", ftell(f1), c1, c1, c2, c2);
    return 0;
}

void do_simple(char * fileName, int algIndx){
    if (algIndx < 0 || (unsigned long)algIndx >= sizeof(algorithms) / sizeof(algorithms[0]))
        return;
    char * compFileName = strcat(realloc(strdup(fileName), strlen(fileName) + strlen(algorithms[algIndx].extension) + 1), algorithms[algIndx].extension);
    FILE * in = fopen(fileName, "rb");
    FILE * out = fopen(compFileName, "wb");

    printf("Compressing %s...\n", fileName);
    clock_t t1 = clock();
    algorithms[algIndx].compress(in, out);
    clock_t t2 = clock();
    double deltaT = (t2 - t1) / (double)CLOCKS_PER_SEC;
    printf("Compressed in %lf s\n", deltaT);

    fseek(in, 0, SEEK_END);
    fseek(out, 0, SEEK_END);

    long input_size = ftell(in), output_size = ftell(out);

    printf("%14s:\n", algorithms[algIndx].name);
    printf(" Original size: %10li Bytes\n", input_size);
    printf("Resulting size: %10li Bytes\n", output_size);
    printf("Resulting comp: %10.3lf%%\n", 100.0 - output_size*100.0/input_size);
    printf("   Total speed: %10.0lf kB/s\n", input_size / deltaT / 1024);
    printf("    Diff speed: %10.0lf kB/s\n", (input_size - output_size) / deltaT / 1024);

    fclose(in);
    fclose(out);

    in = fopen(compFileName, "rb");
    out = fopen("decompressed.temp", "wb");

    printf("Decompressing compressed.temp...\n");
    t1 = clock();
    algorithms[algIndx].decompress(in, out);
    t2 = clock();
    deltaT = (t2 - t1) / (double)CLOCKS_PER_SEC;
    printf("Decompressed in %lf\n", deltaT);
    printf("   Total speed: %10.0lf kB/s\n", input_size / deltaT / 1024);

    fclose(out);
    fclose(in);

    in = fopen(fileName, "rb");
    out = fopen("decompressed.temp", "rb");

    if(are_equal(in, out))
        printf("files are equal\n");
    else
        printf("files are NOT equal\n");

    fclose(out);
    out = fopen(compFileName, "rb");
    rewind(in);
    printf("  Original entropy: %lf\n", measure_entropy(in));
    printf("Compressed entropy: %lf\n\n", measure_entropy(out));

    fclose(in);
    fclose(out);
    free(compFileName);
}

typedef struct fileStruct{
    char fileName[256];
    size_t size;
    double compTitme;
} * fileStruct;

void do_all(char * fileName){
    list fileList = list_init();
    fileStruct ofs = malloc(sizeof(struct fileStruct));
    strcpy(ofs->fileName, fileName);
    ofs->compTitme = 0;
    FILE * f1 = fopen(fileName, "r"), * f2;
    fseek(f1, 0, SEEK_END);
    ofs->size = ftell(f1);
    fclose(f1);
    printf("   Size   |   Comp   |   Time   |   Speed  |  (Diff)  |   Name\n");
    printf("%10li|%10.3lf|%10lf|%10.0lf|%10.0lf|   %s\n", ofs->size, 0.0, 0.0, 0.0, 0.0, ofs->fileName);
    list_append(fileList, ofs);
    for (int i = 0; i < list_length(fileList); i++){
        fileStruct currfs = list_get(fileList, i);
        for (int algIndx = 0; (unsigned long)algIndx < sizeof(algorithms) / sizeof(algorithms[0]); algIndx++){
            fileStruct tempfs = malloc(sizeof(struct fileStruct));
            strcpy(tempfs->fileName, currfs->fileName);
            strcat(tempfs->fileName, algorithms[algIndx].extension);
            f1 = fopen(currfs->fileName, "rb");
            f2 = fopen(tempfs->fileName, "wb");
            clock_t start = clock();
            algorithms[algIndx].compress(f1, f2);
            clock_t end = clock();
            tempfs->compTitme = currfs->compTitme + (end - start) / (double)CLOCKS_PER_SEC;
            fseek(f2, 0, SEEK_END);
            tempfs->size = ftell(f2);
            fclose(f1);
            fclose(f2);

            printf("%10li|%10.3lf|%10lf|%10.0lf|%10.0lf|   %s\n", tempfs->size, 100.0 - tempfs->size*100.0/ofs->size, tempfs->compTitme, ofs->size / tempfs->compTitme / 1024, (ofs->size - tempfs->size) / tempfs->compTitme / 1024, tempfs->fileName);

            if (tempfs->size < currfs->size)
                list_append(fileList, tempfs);
            else
                free(tempfs);
        }
    }
    while(list_length(fileList))
        free(list_pop(fileList));
    list_free(fileList);
}

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