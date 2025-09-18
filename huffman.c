//
// Created by heros on 31/01/25.
//

#include "huffman.h"
#include "lists.h"
#include <stdlib.h>

typedef struct hufftree{
    char contains[256];
    unsigned long int freq;
    int endChar, isEOF;
    struct hufftree * l, * r;
}* hufftree;

int freq_compare_rev(const void * p1, const void * p2){
    const hufftree t1 = p1, t2 = p2;

    return t2->freq - t1->freq;
}

void print_tree(FILE * f, hufftree t){
    fputc('{', f);
    if(t->l == NULL){
        if (t->isEOF){
            fputc('\\', f);
            fputc('e', f);
        }else if (t->endChar == '\\' || t->endChar == '{' || t->endChar == '}'){
            fputc('\\', f);
            fputc(t->endChar, f);
        }else{
            fputc(t->endChar, f);
        }
    }else{
        print_tree(f, t->l);
        print_tree(f, t->r);
    }
    fputc('}', f);
}

void huffman_comp(FILE * source, FILE * dest){
    unsigned long int freq[256] = {0};
    rewind(source);
    int c = fgetc(source);
    while(c != EOF){
        freq[c]++;
        c = fgetc(source);
    }
    list temp = list_init();
    for (int i = 0; i < 256; i++){
        if(freq[i]){
            hufftree t = malloc(sizeof(struct hufftree));
            t->freq = freq[i];
            for(int j = 0; j < 256; j++)
                t->contains[j] = 0;
            t->contains[i] = 1;
            t->l = NULL;
            t->r = NULL;
            t->isEOF = 0;
            t->endChar = i;
            list_ordered_insert(temp, t, freq_compare_rev);
        }
    }

    //add EOF
    hufftree t = malloc(sizeof(struct hufftree));
    t->freq = 1;
    for(int j = 0; j < 256; j++)
        t->contains[j] = 0;
    t->l = NULL;
    t->r = NULL;
    t->isEOF = 1;
    t->endChar = EOF;
    list_append(temp, t);
    t = NULL;

    //make tree
    while (list_length(temp)>1){
        t = list_pop(temp);
        hufftree aux = list_pop(temp);
        hufftree new = malloc(sizeof(struct hufftree));
        new->isEOF = t->isEOF || aux->isEOF;
        new->freq = t->freq + aux->freq;
        new->l = aux;
        new->r = t;
        new->endChar = -2;
        for(int i = 0; i < 256; i++)
            new->contains[i] = t->contains[i] || aux->contains[i];
        list_ordered_insert(temp, new, freq_compare_rev);
    }
    t = list_pop(temp);
    list_free(temp);

    print_tree(dest, t);

    //todo leer valor y conseguir compressión del arbol

    //todo free hufftree
}


void huffman_decomp(FILE * source, FILE * dest){
    //todo
}