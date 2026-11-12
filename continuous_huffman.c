//
// Created by heros on 6/11/26.
//

//an alternative implementation of huffman that builds the tree as it goes
#include <stdlib.h>
#include <stdio.h>
#include "continuous_huffman.h"

#define COUNTBIAS 0x100
#define MEMORY 0x1000

typedef struct{
    size_t bits;
    int bitsLen;
}bitRepl;

bitRepl tree_search(size_t totalCount, size_t countBuff[256], unsigned char goal){
    bitRepl ret = {0};
    int placing = 0;
    int slots = 1;

    while (1){
        if (countBuff[placing] * (slots * 2 - 1) < totalCount){
            //unoptimal so it goes to the next row
            slots *= 2;
            ret.bitsLen++;
            ret.bits <<= 1;
        }else{
            //optimal placement
            slots--;
            if (placing == goal)
                return ret;
            ret.bits++;
            totalCount -= countBuff[placing];
            placing++;
        }
    }
}

void add_to_buffs(size_t * totalCount, size_t countBuff[256], unsigned char charBuff[256],  unsigned char indexBuff[256], int c){
    int index = indexBuff[c];
    int newIndex = index;
    size_t count = countBuff[index];

    //newIndex will be leftmost element of the block
    do
        newIndex--;
    while (newIndex >= 0 && countBuff[newIndex] == count);
    newIndex++;

    //swap newIndex with old index (if necessary)
    if (newIndex != index){
        //Not necesary since counts must bee the same before swap
        //countBuff[index] = countBuff[newIndex];

        unsigned char aux = charBuff[index] = charBuff[newIndex];
        charBuff[newIndex] = c;

        indexBuff[c] = newIndex;
        indexBuff[aux] = index;
    }

    //add COUNTBIAS
    countBuff[newIndex] += COUNTBIAS;
    *totalCount += COUNTBIAS;
}

void sub_to_buffs(size_t * totalCount, size_t countBuff[256], unsigned char charBuff[256],  unsigned char indexBuff[256], int c){
    int index = indexBuff[c];
    int newIndex = index;
    size_t count = countBuff[index];

    //newIndex will be rightmost element of the block
    do
        newIndex++;
    while (newIndex < 256 && countBuff[newIndex] == count);
    newIndex--;

    //swap newIndex with old index (if necessary)
    if (newIndex != index){
        //Not necesary since counts must bee the same before swap
        //countBuff[index] = countBuff[newIndex];

        unsigned char aux = charBuff[index] = charBuff[newIndex];
        charBuff[newIndex] = c;

        indexBuff[c] = newIndex;
        indexBuff[aux] = index;
    }

    //subtract COUNTBIAS
    countBuff[newIndex] -= COUNTBIAS;
    *totalCount -= COUNTBIAS;
}

void continuous_huffman_comp(char * source, char * dest){
    FILE * in = fopen(source, "rb"), * out = fopen(dest, "wb");

    //init buffers
    size_t totalCount = 256;//total amount of counts
    size_t countBuff[256];//the count for each char ordered from most to least
    unsigned char charBuff[256];//the corresponding char for each count
    unsigned char indexBuff[256];//the corresponding index for each char

    for (int i = 0; i < 256; i++){
        countBuff[i] = 1;
        charBuff[i] = indexBuff[i] = i;
    }

    int c = fgetc(in);
    unsigned char byte = 0;
    int bit = 0;
    size_t read = 1;
    unsigned char buffer[MEMORY];
    buffer[0] = c;

    while (c != EOF){
        bitRepl p = tree_search(totalCount, countBuff, indexBuff[c]);

        add_to_buffs(&totalCount, countBuff, charBuff, indexBuff, c);

        while (bit + p.bitsLen >= 8){
            byte |= p.bits >> (p.bitsLen - 8 + bit);
            fputc(byte, out);
            p.bitsLen -= 8 - bit;
            byte = 0;
            bit = 0;
        }
        //save rest
        byte |= p.bits << (8 - p.bitsLen) >> bit;
        bit += p.bitsLen;

        if (read >= MEMORY)
            sub_to_buffs(&totalCount, countBuff, charBuff, indexBuff, buffer[read % MEMORY]);

        buffer[read++ % MEMORY] = c = fgetc(in);
    }

    //print last byte (rellenar con 1 para marcar fin de archivo)
    if (bit)
        fputc(byte | (0xff >> bit), out);

    fclose(in);
    fclose(out);
}

typedef struct{
    unsigned char c;
    int bitsLen;
}charRepl;

charRepl rev_tree_search(size_t totalCount, size_t countBuff[256], unsigned char charBuff[256], size_t goal){
    bitRepl aux = {0};
    int placing = 0;
    int slots = 1;

    while (1){
        if (countBuff[placing] * (slots * 2 - 1) < totalCount){
            //unoptimal so it goes to the next row
            slots *= 2;
            aux.bitsLen++;
            aux.bits <<= 1;
        }else{
            //optimal placement
            slots--;
            if (aux.bits == (goal >> (64 - aux.bitsLen))){
                charRepl ret;
                ret.c = charBuff[placing];
                ret.bitsLen = aux.bitsLen;
                return ret;
            }
            aux.bits++;
            totalCount -= countBuff[placing];
            placing++;
        }
    }
}

void continuous_huffman_decomp(char * source, char * dest){
    FILE * in = fopen(source, "rb"), * out = fopen(dest, "wb");

    //init buffers
    size_t totalCount = 256;//total amount of counts
    size_t countBuff[256];//the count for each char ordered from most to least
    unsigned char charBuff[256];//the corresponding char for each count
    unsigned char indexBuff[256];//the corresponding index for each char

    for (int i = 0; i < 256; i++){
        countBuff[i] = 1;
        charBuff[i] = indexBuff[i] = i;
    }

    int c = fgetc(in);
    size_t bytes = 0;
    int bit = 64;
    size_t read = 0;
    unsigned char buffer[MEMORY];

    while (c != EOF){
        if (bit >= 8){
            bytes |= (size_t)c << (bit - 8);
            bit -= 8;
            c = fgetc(in);
        }else{
            bytes |= c >> (8 - bit); // siempre llenar el fondo

            charRepl r = rev_tree_search(totalCount, countBuff, charBuff, bytes);

            add_to_buffs(&totalCount, countBuff, charBuff, indexBuff, r.c);

            //print char
            fputc(r.c, out);

            buffer[read++ % MEMORY] = r.c;

            if (read >= MEMORY)
                sub_to_buffs(&totalCount, countBuff, charBuff, indexBuff, buffer[read % MEMORY]);

            //remove from bytes
            bytes <<= r.bitsLen;
            bit += r.bitsLen;
        }
    }

    //print last chars
    while (1){
        bytes |= 18446744073709551615 >> (64 - bit); // siempre llenar el fondo tiene que ser ese número raro porque si no lo interpreta como un número negativo

        charRepl r = rev_tree_search(totalCount, countBuff, charBuff, bytes);

        add_to_buffs(&totalCount, countBuff, charBuff, indexBuff, r.c);

        //test if it is final else print char
        if (r.bitsLen > 64 - bit)
            break;
        fputc(r.c, out);

        buffer[read++ % MEMORY] = r.c;

        if (read >= MEMORY)
            sub_to_buffs(&totalCount, countBuff, charBuff, indexBuff, buffer[read % MEMORY]);

        //remove from bytes
        bytes <<= r.bitsLen;
        bit += r.bitsLen;
    }

    fclose(in);
    fclose(out);
}