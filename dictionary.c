//
// Created by heros on 31/01/25.
//

#include "dictionary.h"

#include <ctype.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#include "lists.h"
/*
#define MAXDEPTH 8


typedef struct trie{
    unsigned long amount;
    union{
        struct trie* children[256];
        unsigned long leafAmount[256];
    };
}trie;

void trie_free(trie * t){
    static int depth = 0;
    depth++;
    if (depth < MAXDEPTH)
        for (int i = 0; i < 256; i++)
            if (t->children[i] != NULL)
                continue;
                //trie_free(t->children[i]);
    depth--;
    free(t);
}

trie * trie_init(){
    trie * t = malloc(sizeof(trie));
    t->amount = 0;
    for (int i = 0; i < 256; i++)
        t->children[i] = NULL;
    return t;
}

void rec_print(trie * t){
    static int depth = 0;
    depth++;
    if (depth < MAXDEPTH){
        for (int i = 0; i < 256; i++)
            if (t->children[i] != NULL){
                if (!isprint(i) || i == ' ')
                    printf("%*c%lu \\%i\n", depth * 3, ' ', t->children[i]->amount, i);
                else
                    printf("%*c%lu %c\n", depth * 3, ' ', t->children[i]->amount, i);
                rec_print(t->children[i]);
            }
    }else{
        for (int i = 0; i < 256; i++)
            if (t->leafAmount[i] > 0){
                if (!isprint(i) || i == ' ')
                    printf("%*c%lu \\%i\n", depth * 3, ' ', t->leafAmount[i], i);
                else
                    printf("%*c%lu %c\n", depth * 3, ' ', t->leafAmount[i], i);
            }
    }
    depth--;
}

void reduce(trie * reducedTrieRoot, trie * basicTrieRoot){
    static int depth = 0;

    depth++;
    for (int i = 0; i < 256; i++){
        if (basicTrieRoot->children[i] != NULL){
            if (depth < MAXDEPTH){
                if (basicTrieRoot->children[i]->amount <= 1)
                    continue;
                reducedTrieRoot->children[i] = trie_init();
                reducedTrieRoot->children[i]->amount = basicTrieRoot->children[i]->amount;
                if (reducedTrieRoot->amount == reducedTrieRoot->children[i]->amount)
                    reducedTrieRoot->amount = 0;
                reduce(reducedTrieRoot->children[i], basicTrieRoot->children[i]);
            }else{
                if (basicTrieRoot->leafAmount[i] <= 1)
                    continue;
                reducedTrieRoot->leafAmount[i] = basicTrieRoot->leafAmount[i];
                if (reducedTrieRoot->amount == reducedTrieRoot->leafAmount[i])
                    reducedTrieRoot->amount = 0;
            }
        }
    }
    depth--;
    if (depth <= 1)
        reducedTrieRoot->amount = 0;
}

int search_best(trie * t, char * bestWord, int substLen){
    static int depth = 0;
    static long bestPotential = 0;
    static char currWord[MAXDEPTH] = {'\0'};
    static int bestLen;
    if (depth <= 0){
        bestPotential = 0;
        bestLen = 0;
        for (int i = 0; i < MAXDEPTH; i++){
            bestWord[i] = '\0';
            currWord[i] = '\0';
        }
    }
    depth++;
    for (int i = 0; i < 256; i++){
        if (t->children[i] != NULL){
            currWord[depth-1] = i;
            if (depth < MAXDEPTH){
                if (t->children[i]->amount){
                    long potential = (t->children[i]->amount - 1) * (depth - substLen) - 2 - substLen;
                    if (potential > bestPotential || (potential >= bestPotential && depth < bestLen) ){
                        bestPotential = potential;
                        bestLen = depth;
                        memcpy(bestWord, currWord, MAXDEPTH);
                    }
                }
                search_best(t->children[i], bestWord, substLen);
            }else{
                long potential = (t->leafAmount[i] - 1) * (depth - substLen) - 2 - substLen;
                if (potential > bestPotential){
                    bestPotential = potential;
                    bestLen = depth;
                    memcpy(bestWord, currWord, MAXDEPTH);
                }
            }
            currWord[depth-1] = '\0';
        }
    }
    depth--;
    return bestLen;
}

void rec_remove(trie * t, char * word, int wordLen, unsigned long amount){
    static int startRemove = 0;
    static int depth = 0;
    if (depth == 0)
        startRemove = 0;
    depth++;
    if (depth < wordLen){
        rec_remove(t->children[word[depth-1]], word, wordLen, amount);
        if (startRemove){
            t->children[word[depth-1]] = NULL;
            if (t->amount){
                t->amount -= amount;
                if (t->amount <= 1)
                    trie_free(t);
                else{
                    startRemove = 0;

                    //contr si tiene más de un hijo
                    int children = 0;
                    for (int i = 0; i < 256; i++){
                        if (t->children[i] != NULL)
                            if (children++)
                                break;
                    }
                    if (children <= 1)
                        t->amount = 0;
                }
            }
        }
    }else{
        startRemove = 1;
        if (depth < MAXDEPTH){
            trie_free(t);
        }
    }
    depth--;
}

void remove_word(trie * t, char word[MAXDEPTH], int wordLen){
    static int depth = 0;
    trie * aux = t;
    for (int i = 0; i < wordLen && i < MAXDEPTH; i++)
        aux = aux->children[word[i]];

    unsigned long amount = 0;
    if (wordLen == MAXDEPTH)
        amount = (unsigned long)aux;
    else
        amount = aux->amount;

    rec_remove(t, word, wordLen, amount);
}

void dict_comp(char * source, char * dest){
    //open files
    FILE * in = fopen(source, "rb"), * out = fopen(dest, "wb");

    //exit if null
    if (in == NULL || out == NULL){
        fclose(in);
        fclose(out);
        return;
    }

    //inits
    size_t currByte = 0;
    trie * basicTrieRoot = trie_init();
    trie * tries[MAXDEPTH];
    int c;

    //read MAXDEPTH first bytes
    for (currByte = 0; currByte < MAXDEPTH; currByte++){
        //read byte
        c = fgetc(in);

        //exit if end
        if (c == EOF){
            fclose(in);
            fclose(out);
            trie_free(basicTrieRoot);
            return;
        }

        //fill buffer
        tries[currByte] = basicTrieRoot;

        //fill trie
        for (int i = 0; i <= currByte; i++){
            if (currByte < MAXDEPTH -1 || i > 0){
                if (tries[i]->children[c] == NULL)
                    tries[i] = tries[i]->children[c] = trie_init();
                else
                    tries[i] = tries[i]->children[c];
                tries[i]->amount++;
            }else{
                tries[i]->leafAmount[c]++;
            }
        }
    }

    //read rest of bytes
    do{
        //read byte
        c = fgetc(in);

        //exit if end
        if (c == EOF)
            break;

        //fill buffer
        tries[currByte % MAXDEPTH] = basicTrieRoot;

        //fill trie
        for (int j = 1; j <= MAXDEPTH; j++){
            if (j > 1){
                if (tries[(currByte + j) % MAXDEPTH]->children[c] == NULL)
                    tries[(currByte + j) % MAXDEPTH] = tries[(currByte + j) % MAXDEPTH]->children[c] = trie_init();
                else
                    tries[(currByte + j) % MAXDEPTH] = tries[(currByte + j) % MAXDEPTH]->children[c];
                tries[(currByte + j) % MAXDEPTH]->amount++;
            }else{
                tries[(currByte + j) % MAXDEPTH]->leafAmount[c]++;
            }
        }
    }while (currByte++);

    //printf("Basic Trie:\n");
    //rec_print(basicTrieRoot);

    //redduce trie
    trie * reducedTrieRoot = trie_init();

    reduce(reducedTrieRoot, basicTrieRoot);

    //printf("\n\n\n");
    //printf("Reduced Trie:\n");
    //rec_print(reducedTrieRoot);

    char word[MAXDEPTH+1] = {'\0'};
    int wordLen = 0;


    while (1){
        wordLen = search_best(reducedTrieRoot, word, 1);
        if (wordLen == 0)
            break;
        printf("Best substr:{%s}\n", word);

        remove_word(reducedTrieRoot, word, wordLen);
        
        //printf("Trie with removed substr:\n");
        //rec_print(reducedTrieRoot);
    }

    trie_free(reducedTrieRoot);
    trie_free(basicTrieRoot);
}

void dict_decomp(char * source, char * dest){

}

*/


typedef struct{
    int count;
    char transpose[20];
    char * original;
}dictEntry;

#define BANNED " \n\t{}[]()<=>+-/*%;:.,\"\'\\&!^~|?"

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

void dict_comp(char * source, char * dest){
    FILE * in = fopen(source, "rb"), * out = fopen(dest, "wb");

    char * s = NULL;
    int len = 0;
    int c = fgetc(in);
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

        c = fgetc(in);
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

    fseek(in, 0, SEEK_SET);
    fseek(out, 0, SEEK_SET);

    //escribir en el resultado el diccionario
    for(int i = 0; i < list_length(finalDict); i++){
        dictEntry * p = list_get(finalDict, i);
        fprintf(out, "%s %s ", (char *)p->transpose, (char *)p->original);
    }
    fputc('\n', out);

    //escribir lo traspuesto
    s = NULL;
    len = 0;
    c = fgetc(in);

    while (c != EOF){
        if(isBanned(c)){
            if(s != NULL){
                dictEntry d;
                d.original = s;

                int indx = list_bsearch(finalDict, &d, dict_original_compare);

                if(indx < 0){
                    fputs(s, out);
                }else{
                    dictEntry * p = list_get(finalDict, indx);
                    fputs(p->transpose, out);
                }

                free(s);
                s = NULL;
            }
            fputc(c, out);
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

        c = fgetc(in);
    }

    if(s != NULL){
        dictEntry d;
        d.original = s;

        int indx = list_bsearch(finalDict, &d, dict_original_compare);

        if(indx < 0){
            fputs(s, out);
        }else{
            dictEntry * p = list_get(finalDict, indx);
            fputs(p->transpose, out);
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
    fclose(in);
    fclose(out);
}

void dict_decomp(char * source, char * dest){
    FILE * in = fopen(source, "rb"), * out = fopen(dest, "wb");
    //leer el diccionario
    int c = fgetc(in);
    list dictionary = list_init();
    while(c != '\n'){
        char * s;
        s = malloc(sizeof(char) * 2);
        dictEntry * d = malloc(sizeof(dictEntry));
        s[0] = c;
        s[1] = '\0';
        c = fgetc(in);
        int len = 2;
        //leer primera parte de la entrada
        while(c != ' '){
            s = realloc(s, ++len * sizeof(char));
            s[len-2] = c;
            s[len-1] = '\0';
            c = fgetc(in);
        }
        strcpy(d->transpose, s);
        free(s);
        c = fgetc(in);
        s = malloc(sizeof(char) * 2);
        s[0] = c;
        s[1] = '\0';
        c = fgetc(in);
        len = 2;
        //leer primera parte de la entrada
        while(c != ' '){
            s = realloc(s, ++len * sizeof(char));
            s[len-2] = c;
            s[len-1] = '\0';
            c = fgetc(in);
        }
        d->original = s;
        list_ordered_insert(dictionary, d, dict_trans_compare);
        c = fgetc(in);
    }
    //escribir traduciendo del diccionario
    c = fgetc(in);
    char * s = NULL;
    int len = 0;
    while(c != EOF){
        if(isBanned(c)){
            if(s != NULL){
                dictEntry aux;
                int indx;
                if (strlen(s) < sizeof(aux.transpose)){
                    strcpy(aux.transpose, s);
                    indx = list_bsearch(dictionary, &aux, dict_trans_compare);
                }else
                    indx = -1;
                if(indx < 0){
                    fputs(s, out);
                    free(s);
                    s = NULL;
                }else{
                    aux = *(dictEntry *)list_get(dictionary, indx);
                    fputs(aux.original, out);
                    free(s);
                    s = NULL;
                }
            }
            fputc(c, out);
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
        c = fgetc(in);
    }

    //string final
    if(s != NULL){
        dictEntry aux;
        strcpy(aux.transpose, s);
        int indx = list_bsearch(dictionary, &aux, dict_trans_compare);
        if(indx < 0){
            fputs(s, out);
            free(s);
            s = NULL;
        }else{
            aux = *(dictEntry *)list_get(dictionary, indx);
            fputs(aux.original, out);
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
    fclose(in);
    fclose(out);
}