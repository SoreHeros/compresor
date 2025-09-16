//
// Created by heros on 31/01/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dictionary.h"
#include "rl-encoding.h"
#include "lists.h"

struct{
    char name[16];
    char extension[8];
    void (*compress)(FILE *, FILE *);
    void (*decompress)(FILE *, FILE *);
}algorithms[] = {
        {"dictionary", ".dict", dict_comp, dict_decomp},
        {"Run length", ".rl", rl_compress, rl_decomp}
    };


int main(){
    FILE * f = fopen("bee movie script", "r");
    FILE * out = fopen("bee movie script.temp", "w");
    dict_comp(f, out);

    fseek(f, 0, SEEK_END);
    fseek(out, 0, SEEK_END);

    long input_size = ftell(f), output_size = ftell(out);
    printf("dictionary:\n Original size: %10li\nResulting size: %10li\nResulting comp: %10.3lf%%\n", input_size, output_size, output_size*100.0/input_size);

    return 0;
}