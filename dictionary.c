//
// Created by heros on 31/01/25.
//

#include "dictionary.h"

#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lists.h"

typedef struct{
    int count;
    char transpose[20]; //todo fix lo de que el string de la línea 297 es demasiado grande
    char * original;
}dictEntry;

#define BANNED " \n\t{}[]()<=>+-*/%;:.,\"\'\\&!^~|?"

int isBanned(unsigned char c){
    if(c < 32 || c >= 127)
        return 1;

    char * ptr = strchr(BANNED, c);

    if (ptr == NULL)
        return 0;
    else
        return 1;
}

int dict_original_compare(const void * a, const void * b){
    const dictEntry * d1 = a, * d2 = b;

    return strcmp(d1->original, d2->original);
}

int dict_trans_compare(const void * a, const void * b){
    const dictEntry * d1 = a, * d2 = b;

    return strcmp(d1->transpose, d2->transpose);
}

int dict_count_compare(const void * a, const void * b){
    const dictEntry * d1 = a, * d2 = b;

    return d1->count - d2->count;
}

int transposeLen;

int potential(const dictEntry * d){
    return d->count * (strlen(d->original) - transposeLen) - 2 - strlen(d->original);
}

int dict_potential_compare(const void * a, const void * b){
    const dictEntry * d1 = a, * d2 = b;

    return potential(d1) - potential(d2);
}

void nextComb(char * arr){
    do{
        (*arr)++;
        if(*arr == 0)
            nextComb(arr+1);
    }while(isBanned(*arr));
}

void dict_comp(FILE * source, FILE * dest){
    fseek(source, 0, SEEK_SET);

    char * s = NULL;
    int len = 0;
    int c = fgetc(source);
    list dict = list_init();

    while (c != EOF){
        if(isBanned(c)){
            if(s != NULL){
                //añadir a dict o aumentar en 1
                dictEntry d;
                d.original = s;
                int indx = list_bsearch(dict, &d, dict_original_compare);
                if(indx < 0){
                    dictEntry * new = malloc(sizeof(dictEntry));
                    new->count = 1;
                    new->original = s;
                    new->transpose[0] = '\0';
                    s = NULL;
                    list_ordered_insert(dict, new, dict_original_compare);
                }else{
                    dictEntry * aux = list_get(dict, indx);
                    aux->count++;
                    free(s);
                    s = NULL;
                }
            }
        }else{
            if(s != NULL){
                //realloc y asignar char
                s = realloc(s, ++len * sizeof(unsigned char));
                s[len-2] = c;
                s[len-1] = '\0';
            }else{
                //nuevo malloc
                len = 2;
                s = malloc(sizeof(unsigned char) * 2);
                s[0] = c;
                s[1] = '\0';
            }
        }

        c = fgetc(source);
    }

    //añadir string final
    if(s != NULL){
        //añadir a dict o aumentar en 1
        dictEntry d;
        d.original = s;
        int indx = list_bsearch(dict, &d, dict_original_compare);
        if(indx < 0){
            dictEntry * new = malloc(sizeof(dictEntry));
            new->count = 1;
            new->original = s;
            new->transpose[0] = '\0';
            s = NULL;
            list_ordered_insert(dict, new, dict_original_compare);
        }else{
            dictEntry * aux = list_get(dict, indx);
            aux->count++;
            free(s);
            s = NULL;
        }
    }

    transposeLen = 0;
    char transpose[12] = "\0\0\0\0\0\0\0\0\0\0\0";
    list finalDict = list_init();

    while(1){
        nextComb(transpose);
        dictEntry tmp;
        tmp.original = transpose;
        if(list_bsearch(dict, &tmp, dict_original_compare) != -1)
            continue;
        transposeLen = strlen(transpose);
        int maxPotential = 0;
        dictEntry * p = NULL;

        //buscar maximo potencial
        for(int i = 0; i < list_length(dict); i++){
            dictEntry * aux = list_get(dict, i);
            if(aux->transpose[0] == '\0' && potential(aux) > maxPotential){
                p = aux;
                maxPotential = potential(p);
            }
        }

        //salir si se termina
        if (maxPotential <= 0)
            break;

        memcpy(p->transpose, transpose, 12);

        list_ordered_insert(finalDict, p, dict_original_compare);
    }

    fseek(source, 0, SEEK_SET);
    fseek(dest, 0, SEEK_SET);

    //escribir en el resultado el diccionario
    for(int i = 0; i < list_length(finalDict); i++){
        dictEntry * p = list_get(finalDict, i);
        fprintf(dest, "%s %s ", (char *)p->transpose, (char *)p->original);
    }
    fputc('\n', dest);

    //escribir lo traspuesto
    s = NULL;
    len = 0;
    c = fgetc(source);

    while (c != EOF){
        if(isBanned(c)){
            if(s != NULL){
                dictEntry d;
                d.original = s;

                int indx = list_bsearch(finalDict, &d, dict_original_compare);

                if(indx < 0){
                    fputs(s, dest);
                }else{
                    dictEntry * p = list_get(finalDict, indx);
                    fputs(p->transpose, dest);
                }

                free(s);
                s = NULL;
            }
            fputc(c, dest);
        }else{
            if(s != NULL){
                //realloc y asignar char
                s = realloc(s, ++len * sizeof(unsigned char));
                s[len-2] = c;
                s[len-1] = '\0';
            }else{
                //nuevo malloc
                len = 2;
                s = malloc(sizeof(unsigned char) * 2);
                s[0] = c;
                s[1] = '\0';
            }
        }

        c = fgetc(source);
    }

    if(s != NULL){
        dictEntry d;
        d.original = s;

        int indx = list_bsearch(finalDict, &d, dict_original_compare);

        if(indx < 0){
            fputs(s, dest);
        }else{
            dictEntry * p = list_get(finalDict, indx);
            fputs(p->transpose, dest);
        }

        free(s);
        s = NULL;
    }
    while(list_length(dict)){
        dictEntry * d = list_pop(dict);
        free(d->original);
        free(d);
    }
    list_free(dict);
    list_free(finalDict);
}

void dict_decomp(FILE * source, FILE * dest){
    //leer el diccionario
    rewind(source);
    rewind(dest);
    int c = fgetc(source);
    list dictionary = list_init();
    while(c != '\n'){
        char * s;
        s = malloc(sizeof(char) * 2);
        dictEntry * d = malloc(sizeof(dictEntry));
        s[0] = c;
        s[1] = '\0';
        c = fgetc(source);
        int len = 2;
        //leer primera parte de la entrada
        while(c != ' '){
            s = realloc(s, ++len * sizeof(char));
            s[len-2] = c;
            s[len-1] = '\0';
            c = fgetc(source);
        }
        strcpy(d->transpose, s);
        free(s);
        c = fgetc(source);
        s = malloc(sizeof(char) * 2);
        s[0] = c;
        s[1] = '\0';
        c = fgetc(source);
        len = 2;
        //leer primera parte de la entrada
        while(c != ' '){
            s = realloc(s, ++len * sizeof(char));
            s[len-2] = c;
            s[len-1] = '\0';
            c = fgetc(source);
        }
        d->original = s;
        list_ordered_insert(dictionary, d, dict_trans_compare);
        c = fgetc(source);
    }
    //escribir traduciendo del diccionario
    c = fgetc(source);
    char * s = NULL;
    int len = 0;
    while(c != EOF){
        if(isBanned(c)){
            if(s != NULL){
                dictEntry aux;
                strcpy(aux.transpose, s);
                int indx = list_bsearch(dictionary, &aux, dict_trans_compare);
                if(indx < 0){
                    fputs(s, dest);
                    free(s);
                    s = NULL;
                }else{
                    aux = *(dictEntry *)list_get(dictionary, indx);
                    fputs(aux.original, dest);
                    free(s);
                    s = NULL;
                }
            }
            fputc(c, dest);
        }else{
            if (s == NULL){
                s = malloc(sizeof(char) * 2);
                len = 2;
                s[0] = c;
                s[1] = '\0';
            }else{
                s = realloc(s, sizeof(char)* ++len);
                s[len-2] = c;
                s[len-1] = '\0';
            }
        }
        c = fgetc(source);
    }

    //string final
    if(s != NULL){
        dictEntry aux;
        strcpy(aux.transpose, s);
        int indx = list_bsearch(dictionary, &aux, dict_trans_compare);
        if(indx < 0){
            fputs(s, dest);
            free(s);
            s = NULL;
        }else{
            aux = *(dictEntry *)list_get(dictionary, indx);
            fputs(aux.original, dest);
            free(s);
            s = NULL;
        }
    }

    //liberar mem
    while(list_length(dictionary)){
        dictEntry * d = list_pop(dictionary);
        free(d->original);
        free(d);
    }
    list_free(dictionary);
}