//
// Created by heros on 10/10/25.
//

#include "entropy.h"
#include <math.h>

double measure_entropy(FILE * f){
    rewind(f);

    //initial array of positions
    unsigned long long int arr[256] = {0};

    int c = fgetc(f);

    //count occurences
    while (c != EOF){
        arr[c]++;
        c = fgetc(f);
    }

    //get size
    unsigned long long int size = ftell(f);

    //get entropy
    double ret = 0;

    for (int i = 0; i < 256; i++){
        if (arr[i])
            ret -= (double)arr[i]/size * log2((double)arr[i]/size);
    }

    return ret;
}
