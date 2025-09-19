//
// Created by heros on 19/9/25.
//

#include "sliding_window.h"
#include "lists.h"

#define BUFSIZE 256


void buff_add(char buff[BUFSIZE], int *indx, char c){
    buff[++*indx] = c;
    if (*indx >= BUFSIZE)
        *indx %= BUFSIZE;
}

//returns a list with all the matches
list buff_search(char buff[BUFSIZE], int indx){
    int aux = indx + BUFSIZE + BUFSIZE;
    list l = list_init();
    for (int i = 1; i < BUFSIZE - 3; i++){
        if(buff[(aux - i - 3) % BUFSIZE] == buff[(aux - 3) % BUFSIZE] && buff[(aux - i - 2) % BUFSIZE] == buff[(aux - 2) % BUFSIZE] && buff[(aux - i - 1) % BUFSIZE] == buff[(aux - 1) % BUFSIZE] && buff[(aux - i) % BUFSIZE] == buff[(aux) % BUFSIZE])
            list_append(l, (void *)i);
    }
    if(list_length(l))
        return l;
    list_free(l);
    return NULL;
}


void buff_match_last_from_list(char buff[BUFSIZE], int indx, list match){
    list apt = list_init();

    //get apt
    for (int i = 0; i < list_length(match); i++)
        if (buff[(indx + BUFSIZE - (int)list_get(match, i)) % BUFSIZE] == buff[indx])
            list_append(apt, list_get(match, i));

    //empty list
    while (list_length(match))
        list_pop(match);

    //replace with apt
    while (list_length(apt))
        list_append(match, list_pop(apt));
}

void sw_compress(FILE * source, FILE * dest){
    char buff[BUFSIZE] = {0};
    int indx = -1;
    list match = NULL;
    int c = fgetc(source);
    int maxlen, maxindx;
    int count = 0;
    while (c != EOF){
        count++;
        buff_add(buff, &indx, c);

        if(match == NULL){
            if (count >= 3){
                count = 3;
                match = buff_search(buff, indx);
                if (match != NULL){
                    fseek(dest, -3, SEEK_CUR);
                    maxlen = 4;
                }
            }
        }else{
            maxindx = (int)list_get(match, 0);
            buff_match_last_from_list(buff, indx, match); //teóroicamente se podría cambiar para que no intente siempre comprimir de primeras, si no que si encuentra una compresión mejor utilizando otro de los 3 primeros distintos
            if(list_length(match) && maxlen < 256)
                maxlen++;
            else{
                fputc('\\', dest);
                fputc(maxindx, dest);
                fputc(maxlen, dest);
                list_free(match);
                match = NULL;
                count = 0;
            }
        }

        if (match == NULL){
            if (c == '\\')
                fputc('\\', dest);
            fputc(c, dest);
        }


        c = fgetc(source);
    }
    //escribir último buffer
    if (match != NULL){
        fputc('\\', dest);
        fputc(maxindx, dest);
        fputc(maxlen, dest);
        list_free(match);
    }
}
void sw_decompress(FILE * source, FILE * dest){
    char buff[BUFSIZE] = {0};
}
