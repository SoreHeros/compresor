//
// Created by heros on 19/9/25.
//

#include "sliding_window.h"

#include <stdlib.h>
#include <string.h>

#include "lists.h"


#define LENGTHBITS 7
#define OFFSETBITS 16
#define BUFFERSIZE (0b1 << OFFSETBITS)
#define LENGTHSIZE (0b1 << LENGTHBITS)
#define MINMATCH   (2 + (OFFSETBITS + LENGTHBITS) / 8 + !!((OFFSETBITS + LENGTHBITS) % 8))
#define CLEANUPTIME BUFFERSIZE

typedef struct buffer{
    unsigned char buff[BUFFERSIZE];
    long long int indx, last_written;
    list listBuff[256*256];
}* buffer;

buffer buffer_init(){
    buffer b = malloc(sizeof(struct buffer));
    b->indx = -1;
    b->last_written = -1;
    for (int i = 0; i < 256*256; i++)
        b->listBuff[i] = list_init();
    return b;
}

void buffer_free(buffer b){
    for (int i = 0; i < 256*256; i++){
            list_free(b->listBuff[i]);
            b->listBuff[i] = NULL;
        }
    b->indx = 0;
    b->last_written = 0;
    free(b);
}

void read_to_buffer(buffer b, int c){
    b->buff[++b->indx % BUFFERSIZE] = c;

    if (b->indx >= MINMATCH){
        unsigned char c1, c2;
        c1 = b->buff[(b->indx - MINMATCH) % BUFFERSIZE];
        c2 = b->buff[(b->indx - MINMATCH + 1) % BUFFERSIZE];
        list_append(b->listBuff[c1 * 256 + c2], (void *)(b->indx - MINMATCH));
    }
}

extern unsigned char * get_min_match(buffer b){
    static unsigned char match[MINMATCH];
    if (b->indx >= MINMATCH + b->last_written){
        for (int i = 0; i < MINMATCH; i++)
            match[i] = b->buff[(b->indx - MINMATCH + i + 1) % BUFFERSIZE];
        return match;
    }
    return NULL;
}

list search_match(buffer b, unsigned char * match){
    //get listIndx
    int listIndx = match[0] * 256 + match[1];

    //purge old list entries if necesary
    list old = b->listBuff[listIndx];
    list new;
    if ((long long int)list_get(old, 0) <= b->indx - BUFFERSIZE){
        //purge necessary
        new = list_init();
        int i = 1;
        for (; i < list_length(old); i++)
            if ((long long int)list_get(old, i) > b->indx - BUFFERSIZE)
                break;
        for (; i < list_length(old); i++)
            list_append(new, list_get(old, i));
        list_free(old);
        b->listBuff[listIndx] = new;
    }else{
        //purge not necessary
        new = old;
    }

    //get only actual matches
    list matchList = list_init();
    for (int i = 0; i < list_length(new); i++){
        long long int indx = (long long int)list_get(new, i);
        for (int j = 2; j < MINMATCH; j++)
            if (match[j] != b->buff[(indx + j)%BUFFERSIZE])
                goto SKIP;
        list_append(matchList, (void *)(b->indx - indx - MINMATCH + 1));
        SKIP:
        continue;
    }

    return matchList;
}

list expand_match(list l, buffer b){
    list new = list_init();
    for (int i = 0; i < list_length(l); i++){
        long long int offset = (long long int)list_get(l, i);
        if (b->buff[b->indx % BUFFERSIZE] == b->buff[(b->indx - offset) % BUFFERSIZE])
            list_append(new, (void *)offset);
    }
    return new;
}

void write_match(list l, buffer b, FILE * f){
    long long int offset = (long long int)list_get(l, list_length(l)-1);

    fputc('\\', f);
    fputc(b->indx - 1 - b->last_written + (offset > 0xff ? 0x80 : 0x00), f);//length va primero porque si length es 0 entonces significa que es el caracter de control por si mismo, y el bit superior significa si el offset mide 1 o 2 bytes
    fputc(offset & 0xff, f);
    if (offset > 0xff)
        fputc(offset >> 8, f); //escribir solo si es necesario

    b->last_written = b->indx - 1;
}

void write_last_match(list l, buffer b, FILE * f){
    long long int offset = (long long int)list_get(l, list_length(l)-1);

    fputc('\\', f);
    fputc(b->indx - b->last_written + (offset > 0xff ? 0x80 : 0x00), f);//length va primero porque si length es 0 entonces significa que es el caracter de control por si mismo, y el bit superior significa si el offset mide 1 o 2 bytes
    fputc(offset & 0xff, f);
    if (offset > 0xff)
        fputc(offset >> 8, f); //escribir solo si es necesario

    b->last_written = b->indx;
}

void write_normal(buffer b, FILE * f){
    if (b->last_written < b->indx - MINMATCH){
        unsigned char c = b->buff[++b->last_written % BUFFERSIZE];
        fputc(c, f);
        if (c == '\\')//hacer que el tamaño dependa del tamaño de LENGTHBITS
            fputc('\0', f);
    }
}

void write_last(buffer b, FILE * f){
    while (b->last_written < b->indx){
        unsigned char c = b->buff[++b->last_written % BUFFERSIZE];
        fputc(c, f);
        if (c == '\\')//hacer que el tamaño dependa del tamaño de LENGTHBITS
            fputc('\0', f);
    }
}

void buffer_add(buffer b, int c);
void buffer_write();
void buffer_write_last();

void sw_compress(FILE * source, FILE * dest){
    rewind(source);
    rewind(dest);

    buffer b = buffer_init();
    int c = fgetc(source);
    list l = NULL;
    int len = 0;
    while(c != EOF){
        read_to_buffer(b, c);
        if (l != NULL){
            list temp = expand_match(l, b);
            len++;
            if (!list_length(temp) || len == LENGTHSIZE){
                write_match(l, b, dest);
                list_free(l);
                list_free(temp);
                l = NULL;
            }else{
                list_free(l);
                l = temp;
            }
        }else{
            unsigned char * match = get_min_match(b);
            if (match != NULL){
                l = search_match(b, match);
                match = NULL;
                len = MINMATCH;
                if(!list_length(l)){
                    list_free(l);
                    l = NULL;
                }
            }
            write_normal(b, dest);
        }
        c = fgetc(source);
    }
    if (l != NULL){
        write_last_match(l, b, dest);
        list_free(l);
        l = NULL;
    }else
        write_last(b, dest);
    buffer_free(b);
}

void long_write(unsigned char b[BUFFERSIZE], long long int * indx, int len, int offset, FILE * dest){
    for (int i = 0; i < len; i++){
        unsigned char c = b[(*indx - offset + 1) % BUFFERSIZE];
        b[++*indx % BUFFERSIZE] = c;
        fputc(c, dest);
    }
}

void sw_decompress(FILE * source, FILE * dest){
    unsigned char b[BUFFERSIZE];
    long long int indx = -1;
    int c = fgetc(source);
    while (c != EOF){
        if (c == '\\'){
            int len = fgetc(source);
            if (len == 0){
                //caso de escribir '\'
                b[++indx % BUFFERSIZE] = '\\';
                fputc('\\', dest);
            }else{
                int offset = fgetc(source);
                if (len & 0x80){
                    len &= ~0x80;
                    offset |= fgetc(source) << 8;
                }
                long_write(b, &indx, len, offset, dest);
            }
        }else{
            b[++indx % BUFFERSIZE] = c;
            fputc(c, dest);
        }
        c = fgetc(source);
    }
}