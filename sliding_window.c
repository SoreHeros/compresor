//
// Created by heros on 19/9/25.
//

#include "sliding_window.h"

#include <stdlib.h>
#include <string.h>
#include <asm-generic/errno.h>

#include "lists.h"


#define LENGTHBITS 8
#define OFFSETBITS 16
#define BUFFERSIZE (0b1 << OFFSETBITS)
#define LENGTHSIZE (0b1 << LENGTHBITS)
#define MINMATCH   (2 + (OFFSETBITS + LENGTHBITS) / 8 + !!((OFFSETBITS + LENGTHBITS) % 8))
#define CLEANUPTIME BUFFERSIZE

typedef struct{
    int length;
    int offset;
}pair;

typedef struct trie{
    int filled;
    union{
        struct trie * tries[256];
        list lists[256];
    };
}* trie;

typedef struct buffer{
    unsigned char buff[BUFFERSIZE];
    long long int indx, last_written;
    trie t;
}* buffer;

trie trie_init(){
    trie t = malloc(sizeof(struct trie));
    t->filled = 0;
    for (int i = 0; i < 256; i++)
        t->tries[i] = NULL;
    return t;
}

void add_match(trie t, unsigned char * match, unsigned long long int indx, int matchLen){
    if (matchLen <= 1){
        if (t->lists[*match] == NULL){
            t->filled++;
            t->lists[*match] = list_init();
        }
        list_append(t->lists[*match], (void *)indx);
    }else{
        if (t->tries[*match] == NULL){
            t->filled++;
            t->tries[*match] = trie_init();
        }
        add_match(t->tries[*match], match+1, indx, matchLen-1);
    }
}

void remove_match(trie t, unsigned char * match, int matchLen){
    if (matchLen <= 1){
        list_remove(t->lists[*match], 0);
        /*
        if (list_length(t->lists[*match]) == 0){
            list_free(t->lists[*match]);
            t->lists[*match] = NULL;
            t->filled--;
        }*/
    }else{
        remove_match(t->tries[*match], match+1, matchLen-1);
        /*
        if (t->tries[*match]->filled == 0){
            free(t->tries[*match]);
            t->tries[*match] = NULL;
            t->filled--;
        }*/
    }
}

buffer buffer_init(){
    buffer b = malloc(sizeof(struct buffer));
    b->indx = -1;
    b->last_written = -1;
    b->t = trie_init();
    return b;
}

void trie_full_free(trie t, int depth){
    if (t == NULL)
        return;
    if (depth <= 1){
        for (int i = 0; i < 256; i++)
            if (t->lists[i] != NULL)
                list_free(t->lists[i]);
    }else{
        for (int i = 0; i < 256; i++){
            trie_full_free(t->tries[i], depth-1);
            free(t->tries[i]);
        }
    }
}

void buffer_free(buffer b){
    trie_full_free(b->t, MINMATCH);
    free(b->t);
    b->t = NULL;
    b->indx = 0;
    b->last_written = 0;
    free(b);
}

list get_match(trie t, unsigned char * match, int matchLen){
    if (t == NULL)
        return NULL;
    if (matchLen <= 1){
        return t->lists[*match];
    }
    return get_match(t->tries[*match], match+1, matchLen-1);
}

void read_to_buffer(buffer b, int c){
    if (b->indx >= BUFFERSIZE - 1){
        unsigned char match[MINMATCH];
        for (int i = 0; i < MINMATCH; i++)
            match[i] = b->buff[(b->indx + i + 1) % BUFFERSIZE];
        remove_match(b->t, match, MINMATCH);
    }

    b->buff[++b->indx % BUFFERSIZE] = c;

    if (b->indx >= MINMATCH){
        unsigned char match[MINMATCH];
        for (int i = 0; i < MINMATCH; i++)
            match[i] = b->buff[(b->indx - MINMATCH + i) % BUFFERSIZE];
        add_match(b->t, match, b->indx - MINMATCH, MINMATCH);
    }
}

unsigned char * get_min_match(buffer b){
    if (b->indx > MINMATCH + b->last_written){
        unsigned char * match = malloc(sizeof(unsigned char) * MINMATCH);
        for (int i = 0; i < MINMATCH; i++)
            match[i] = b->buff[(b->indx - MINMATCH + i + 1) % BUFFERSIZE];
        return match;
    }
    return NULL;
}

list search_match(buffer b, unsigned char * match){
    list aux = get_match(b->t, match, MINMATCH);
    list l = list_init();
    if (aux == NULL)
        return l;
    for (int i = 0; i < list_length(aux); i++){
        unsigned long long int indx = (unsigned long long int)list_get(aux, i);
        pair p = {MINMATCH, b->indx - indx - MINMATCH + 1};
        list_append(l, *(void **)&p);
    }
    return l;
}

list expand_match(list l, buffer b){
    list new = list_init();
    for (int i = 0; i < list_length(l); i++){
        void * aux = list_get(l, i);
        pair p = *(pair *)&aux;
        if (b->buff[b->indx % BUFFERSIZE] == b->buff[(b->indx - p.offset) % BUFFERSIZE]){
            p.length++;
            list_append(new, *(void**)&p);
        }
    }
    return new;
}

void write_match(list l, buffer b, FILE * f){
    void * aux = list_get(l, list_length(l)-1);
    pair p = *(pair *)&aux;

    //todo hacer que escriba dependiendo del número de bits
    fputc('\\', f);
    fputc(p.length, f);//length va primero porque si length es 0 entonces significa que es el caracter de control por si mismo
    fputc(p.offset >> 8, f);
    fputc(p.offset & 0xff, f);

    b->last_written += p.length;
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
                free(match);
                match = NULL;
                len = 0;
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
        write_match(l, b, dest);
        list_free(l);
        l = NULL;
    }else
        write_last(b, dest);
    buffer_free(b);
}
void sw_decompress(FILE * source, FILE * dest){

}