//
// Created by heros on 10/10/25.
//

#include "entropy.h"
#include <math.h>

#define BUFFSIZ 0x10000

double measure_entropy(char * fileName){
    FILE * f = fopen(fileName, "rb");
    size_t arr[256] = {0};
    unsigned char buff[BUFFSIZ];

    //count occurences
    size_t read;
    do{
        read = fread(buff, 1, BUFFSIZ, f);
        for (size_t i = 0; i < read; i++){
            arr[buff[i]]++;
        }
    }while (read == BUFFSIZ);

    //get size
    size_t size = ftell(f);

    //get entropy
    double ret = 0;

    for (int i = 0; i < 256; i++){
        if (arr[i])
            ret -= (double)arr[i]/size * log2((double)arr[i]/size);
    }

    fclose(f);
    return ret;
}
