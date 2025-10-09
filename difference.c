//
// Created by heros on 9/10/25.
//

#include "difference.h"

void difference_comp(FILE * source, FILE * dest){
    rewind(source);
    rewind(dest);


    int c = fgetc(source);
    int prev = 0;

    while(c != EOF){
        fputc(c - prev, dest);
        prev = c;
        c = fgetc(source);
    }
}

void difference_decomp(FILE * source, FILE * dest){
    rewind(source);
    rewind(dest);

    int diff = fgetc(source);
    int carry = 0;

    while (diff != EOF){
        carry += diff;
        fputc(carry & 0xff, dest);
        diff = fgetc(source);
    }
}