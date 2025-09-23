//
// Created by heros on 31/01/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dictionary.h"
#include "huffman.h"
#include "lists.h"
#include "sliding_window.h"

struct{
    char name[16];
    char extension[8];
    void (*compress)(FILE *, FILE *);
    void (*decompress)(FILE *, FILE *);
}algorithms[] = {
        {"dictionary", ".dict", dict_comp, dict_decomp},
        {"huffman", ".hfmn", huffman_comp, huffman_decomp},
        {"sliding window", ".slwi", sw_compress, sw_decompress}
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

int main(int len, char ** arr){
    char * original = "main.c";
    int alg = 1;
    if (len > 1)
        alg = atoi(arr[1]);

    if(len > 2)
        original = arr[2];

    FILE * in = fopen(original, "rb");
    FILE * out = fopen("compressed.temp", "wb");

    printf("Compressing %s...\n", original);
    clock_t t1 = clock();
    algorithms[alg].compress(in, out);
    clock_t t2 = clock();
    printf("Compressed in %lf\n", (t2 - t1) / (double)CLOCKS_PER_SEC);

    fseek(in, 0, SEEK_END);
    fseek(out, 0, SEEK_END);

    long input_size = ftell(in), output_size = ftell(out);
    printf("%s:\n Original size: %10li\nResulting size: %10li\nResulting comp: %10.3lf%%\n", algorithms[alg].name,input_size, output_size, 100.0 - output_size*100.0/input_size);

    fclose(in);
    fclose(out);

    in = fopen("compressed.temp", "rb");
    out = fopen("decompressed.temp", "wb");

    printf("Decompressing compressed.temp...\n");
    t1 = clock();
    algorithms[alg].decompress(in, out);
    t2 = clock();
    printf("Decompressed in %lf\n", (t2 - t1) / (double)CLOCKS_PER_SEC);

    fclose(out);
    fclose(in);

    in = fopen(original, "rb");
    out = fopen("decompressed.temp", "rb");

    if(are_equal(in, out))
        printf("files are equal\n");
    else
        printf("files are NOT equal\n");

    fclose(in);
    fclose(out);
    return 0;
}