//
// Created by heros on 31/01/25.
//

#include "huffman.h"
#include "lists.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFSIZ 0x10000

typedef struct hufftree{
    size_t freq;
    int c, isLeaf;
    struct hufftree * l, * r;
}* hufftree;

typedef struct repl{
    size_t bits, size;
}repl;

int freq_compare_rev(const void * p1, const void * p2){
    return (int)(((hufftree)p2)->freq - ((hufftree)p1)->freq);
}

void free_hufftree(hufftree t){
    if (t == NULL)
        return;

    free_hufftree(t->l);
    free_hufftree(t->r);
    free(t);
}

//todo fix
void print_tree_and_size(FILE * f, list depthList, size_t size){
    {
        //se imprimirá en grupos de 7 bits y el primer bit dirá si es el último bloque del grupo o no, siempre debe de haber al menos un bloque que sea el último(bit 0), se utilizarán los 2 primeros bits del siguiente byte para decir el tamaño

        //edge case que literalmente nunca se va a necesitar (un archivo de más de 254 petabytes)
        int started = 0;
        if (size & 0xfc00000000000000){
            started = 1;
            fputc(0x80 | (int)((size & 0xfc00000000000000) >> 58), f);
        }

        int bit = 51;

        while (bit >= 9){
            size_t mask = (size_t)0x7f << bit;
            int c = 0x80;
            if (size & mask){
                started = 1;
                c |= (size & mask) >> bit;
            }
            if (started)
                fputc(c, f);
            bit -= 7;
        }

        //last section is always written even if empty
        fputc((int)(size & 0x01fc) >> 2, f);
    }

    //buscar profundidad de n;
    for (int i = 1; i < list_length(depthList); i++){
        list temp = list_get(depthList, i);
        for (int j = 0; j < list_length(temp); j++){
            int c = ((hufftree)list_get(temp, j))->c;
            if (c == '\n'){
                fputc((i-1) | (int)(size & 0x3) << 6, f);
                goto SKIP;
            }

        }
    }
    //si no existe \n
    fputc(0x3f | (int)(size & 0x3) << 6, f);
    SKIP:

    //skip first
    for (int i = 1; i < list_length(depthList); i++){
        list temp = list_get(depthList, i);
        for (int j = 0; j < list_length(temp); j++){
            int c = ((hufftree)list_get(temp, j))->c;
            fputc(c, f);
        }
        if (i < list_length(depthList)-1)
            fputc('\n', f); //saltarse último
    }
}

hufftree build_huftree(const size_t freq[256]){
    list l = list_init();
    for (int i = 0; i < 256; i++)
        if (freq[i] != 0){
            hufftree t = malloc(sizeof(struct hufftree));
            t->freq = freq[i];
            t->c = i;
            t->isLeaf = 1;
            t->l = NULL;
            t->r = NULL;

            list_ordered_insert(l, t, freq_compare_rev);
        }

    //fuse tree
    hufftree t;
    while (list_length(l) > 1){
        hufftree t1 = list_pop(l);
        hufftree t2 = list_pop(l);

        t = malloc(sizeof(struct hufftree));
        t->freq = t1->freq + t2->freq;
        t->isLeaf = 0;
        t->l = t2;
        t->r = t1;

        list_ordered_insert(l, t, freq_compare_rev);
    }

    t = list_pop(l);

    list_free(l);

    return t;
}

void rec_unwrap(list l, hufftree t, int depth){
    if (t == NULL)
        return;

    if (list_length(l) <= depth)
        list_append(l, list_init());

    if (t->isLeaf){
        list temp = list_get(l, depth);
        list_append(temp, t);
    }else{
        rec_unwrap(l, t->l, depth+1);
        rec_unwrap(l, t->r, depth+1);
    }
}

void rec_write_arr(hufftree t, repl replArr[256], int bits, int depth){
    if (t == NULL)
        return;

    if (t->isLeaf){
        replArr[t->c].bits = bits;
        replArr[t->c].size = depth;
    }else{
        rec_write_arr(t->l, replArr, bits, depth + 1);
        rec_write_arr(t->r, replArr, bits | 0b1 << depth, depth + 1);
    }
}

list build_depth_list(hufftree t){
    list ret = list_init();

    //unwrap
    rec_unwrap(ret, t, 0);

    return ret;
}

void build_replArr(list depthList, repl replarr[256]){
    size_t bits = 0;

    for (int i = 1; i < list_length(depthList); i++){
        list aux = list_get(depthList, i);
        for (int j = 0; j < list_length(aux); j++){
            int c = ((hufftree)list_get(aux, j))->c;
            //make revbits
            size_t revBits = 0;
            for (int k = 0; k < i; k++)
                revBits |= (0b1 & bits >> (i-1-k)) << k;

            replarr[c].bits = revBits;
            bits++;
            replarr[c].size = i;
        }
        bits <<= 1;
    }
}

//todo separate into functs
void huffman_comp(char * source, char * dest){
    FILE * in = fopen(source, "rb"), * out = fopen(dest, "wb");
    unsigned char inBuff[BUFFSIZ], outBuff[BUFFSIZ];

    //read character frequency
    size_t freq[256] = {0};
    size_t read;
    do{
        read = fread(inBuff, 1, BUFFSIZ, in);
        for (size_t i = 0; i < read; i++){
            freq[inBuff[i]]++;
        }
    }while (read == BUFFSIZ);

    //return if file is empty
    if (ftell(in) == 0){
        fclose(in);
        fclose(out);
        return;
    }

    size_t size = ftell(in);

    rewind(in);

    hufftree t = build_huftree(freq);

    //build replace arr
    repl replArr[256];
    list depthList = build_depth_list(t);

    //print hufftree and size
    print_tree_and_size(out, depthList, size);

    //make replArrSS
    build_replArr(depthList, replArr);
    free_hufftree(t);
    while (list_length(depthList))
        list_free(list_pop(depthList));
    list_free(depthList);

    //debug print
    //for (int i = 0; i < 256; i++)
    //    if (replArr[i].size){
    //        printf("%c(%i):\t%2lu\t", i, i, replArr[i].size);
    //        for (size_t j = 0; j < replArr[i].size; j++)
    //            printf("%li", (replArr[i].bits & 0b1 << j) >> j);
    //        printf("\n");
    //    }

    //do actual compression
    size_t writing = 0;
    size_t byte = 0, bit = 0;
    do{
        read = fread(inBuff, 1, BUFFSIZ, in);
        for (size_t i = 0; i < read; i++){
            repl r = replArr[inBuff[i]];
            byte |= r.bits << bit;
            bit += r.size;
            while (bit >= 8){
                //write byte and buffer if necessary
                outBuff[writing++] = byte;
                byte >>= 8;
                bit -= 8;
                if (writing >= BUFFSIZ){
                    fwrite(outBuff, BUFFSIZ, 1, out);
                    writing = 0;
                }
            }
        }
    }while (read == BUFFSIZ);

    //finish last byte
    if (bit){
        outBuff[writing++] = byte;
    }

    //write last buffer
    fwrite(outBuff, writing, 1, out);

    fclose(in);
    fclose(out);
}

typedef struct lookup{
    union{
        short int indx;
        unsigned char c;
    };
    short int bits; // if bits is 0 do normal lookup using indx, if not use c as resulting char discarding bits number of bits
}lookup;

//fills depthArr, charArr and returns size
size_t read_tree(int depthArr[64], unsigned char charArr[256], FILE * in){
    int charCount = 0;
    int totalCount = 2;
    int depth = 1;
    int c, newLinePos;
    size_t size;
    depthArr[0] = 0;

    {//read size and newLinePos
        size = 0;
        do{
            c = fgetc(in);
            size <<= 7;
            size |= c & 0x7f;
        }while (c >= 0x80);

        c = fgetc(in);
        size <<= 2;
        size |= (c & 0b11000000) >> 6;
        newLinePos = c & 0b00111111;
    }

    //read tree
    do{
        c = fgetc(in);
        if (c == '\n'){
            if (!newLinePos--)
                charArr[charCount++] = '\n';
            else{
                depthArr[depth] = charCount;
                totalCount += totalCount - depthArr[depth];
                depth++;
            }
        }else
            charArr[charCount++] = c;
    }while (charCount < totalCount);

    depthArr[depth] = charCount;

    //debug print
    //printf("depthArr: ");
    //for (int i = 0; i <= depth; i++)
    //    printf("%3i ", depthArr[i]);
    //printf("\n");
    //printf("charArr: ");
    //for (int i = 0; i < charCount; i++)
    //    printf("%c ", charArr[i]);
    //printf("\n");

    return size;
}

lookup * do_lookup(int depthArr[64], unsigned char charArr[256]){
    short int len = 256, lindex = 0;
    lookup * l = calloc(len, sizeof(lookup));

    for (int bits = 1, carry = 0, indx = 0; bits <= 8; bits++){
        if (depthArr[bits]){
            for (int i = depthArr[bits-1]; i < depthArr[bits]; i++){
                char c = charArr[indx++];
                int revCarry = 0;
                for (int k = 0; k < bits; k++)
                    revCarry |= (0b1 & carry >> (bits-1-k)) << k;
                for (int j = revCarry; j < len; j += 0b1 << bits){
                    l[j].bits = bits;
                    l[j].c = c;
                }
                carry++;
            }
        }
        carry <<= 1;
    }

    return l;
}

void huffman_decomp(char * source, char * dest){
    FILE * in = fopen(source, "rb"), * out = fopen(dest, "wb");
    unsigned char inBuff[BUFFSIZ], outBuff[BUFFSIZ];
    size_t reading, writing, leftover;
    size_t size;
    lookup * l;

    {//read tree and make lookup table
        int depthArr[64];
        unsigned char charArr[256];
        size = read_tree(depthArr, charArr, in);
        l = do_lookup(depthArr, charArr);
    }

    //decompress
    leftover = size;
    reading = 0;
    writing = 0;
    int index, depth, bits, count, c;
    fread(inBuff, sizeof(char), BUFFSIZ, in);
    index = 0;
    depth = 0;
    bits = 0;
    count = 0;
    while(leftover > 0){
        if (count < 8){
            if (reading >= BUFFSIZ){
                fread(inBuff, sizeof(char), BUFFSIZ, in);
                reading = 0;
            }
            bits |= inBuff[reading++] << count;
            count += 8;
        }

        if (l[bits & 0xff].bits){
            c = l[bits & 0xff].c;
            count -= l[bits & 0xff].bits;
            bits >>= l[bits & 0xff].bits;
        }else{
            //todo fix index and non lookup
            index = l[bits & 0xff].indx;
            bits >>= 8;
            count -= 8;
            depth = 8;

            do{
                if (!count){
                    count = 8;
                    if (reading >= BUFFSIZ){
                        fread(inBuff, sizeof(char), BUFFSIZ, in);
                        reading = 0;
                    }
                    bits = inBuff[reading++];
                }
                //index = 2 * index - depthArr[depth] + (bits & 0b1);
                depth++;
                bits >>= 1;
                count--;
            }while (0 /*index >= depthArr[depth]*/);

            //c = charArr[index];
        }

        if (writing >= BUFFSIZ){
            fwrite(outBuff, sizeof(char), BUFFSIZ, out);
            writing = 0;
        }
        outBuff[writing++] = c;
        leftover--;
    }

    fwrite(outBuff, sizeof(char), writing, out);

    free(l);
    fclose(in);
    fclose(out);
}