//
// Created by heros on 9/10/25.
//

#include "difference.h"

#define BUFFSIZ 0x10000

void difference_comp(char * source, char * dest){
    FILE * in = fopen(source, "rb"), * out = fopen(dest, "wb");
    char inbuff[BUFFSIZ], outbuff[BUFFSIZ];

    size_t read;
    char prev = 0;

    do{
        read = fread(inbuff, 1, BUFFSIZ, in);
        outbuff[0] = inbuff[0] - prev;
        for (size_t i = 1; i < read; i++){
            outbuff[i] = inbuff[i] - inbuff[i-1];
        }
        prev = inbuff[BUFFSIZ - 1];
        fwrite(outbuff, 1, read, out);
    }while (read == BUFFSIZ);
    fclose(in);
    fclose(out);
}

void difference_decomp(char * source, char * dest){
    FILE * in = fopen(source, "rb"), * out = fopen(dest, "wb");
    char inbuff[BUFFSIZ], outbuff[BUFFSIZ];

    size_t read;
    char prev = 0;

    do{
        read = fread(inbuff, 1, BUFFSIZ, in);
        outbuff[0] = inbuff[0] + prev;
        for (size_t i = 1; i < read; i++){
            outbuff[i] = inbuff[i] + outbuff[i-1];
        }
        prev = outbuff[BUFFSIZ - 1];
        fwrite(outbuff, 1, read, out);
    }while (read == BUFFSIZ);

    fclose(in);
    fclose(out);
}