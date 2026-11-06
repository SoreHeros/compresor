//
// Created by heros on 6/11/26.
//

//an alternative implementation of huffman that builds the tree as it goes

#include "continuous_huffman.h"
#include <stdio.h>

#define NULLREF -1
#define HUFFTREESIZE (256 * 2 - 1)

typedef struct{
    size_t count;
    short int parent, isRight, child[2];
}huffTreeEntry;

void init_hufftree(huffTreeEntry tree[HUFFTREESIZE]){
    int i = 0;

    //leaf nodes
    do{
        tree[i].child[0] = NULLREF;
        tree[i].child[1] = NULLREF;
        tree[i].count = 0;
        //tree[i].count = 1;
        tree[i].parent = 256 + i / 2;
        tree[i].isRight = i % 2;
    }while (++i < 256);

    //regular nodes
    do{
        tree[i].child[0] = 2 * (i - 256);
        tree[i].child[1] = 2 * (i - 256) + 1;
        tree[i].count = 0;
        //tree[i].count = tree[tree[i].child[0]].count + tree[tree[i].child[1]].count;
        tree[i].parent = 256 + i / 2;
        tree[i].isRight = i % 2;
    }while (++i < HUFFTREESIZE);

    //root node
    tree[HUFFTREESIZE - 1].parent = NULLREF;
}

void swap(huffTreeEntry tree[HUFFTREESIZE], int a, int b){
    //aux for swaps
    huffTreeEntry aux = tree[a];
    huffTreeEntry bux = tree[b];

    //swap child refs
    tree[aux.parent].child[aux.isRight] = b;
    tree[bux.parent].child[bux.isRight] = a;

    //swap parent refs
    tree[a].parent = bux.parent;
    tree[b].parent = aux.parent;

    //swaps isRights
    tree[a].isRight = bux.isRight;
    tree[b].isRight = aux.isRight;
}

void add_hufftree(huffTreeEntry tree[HUFFTREESIZE], int c){
    tree[c].count++;

    //climb as high as possible untill root
    while (tree[tree[c].parent].parent != NULLREF){
        int parent = tree[c].parent;
        int uncle = tree[tree[parent].parent].child[!tree[parent].isRight];
        if (tree[c].count > tree[uncle].count){
            //swap if is bigger than parent's brother
            swap(tree, c, uncle);
        }else{
            //else switch to parent
            c = tree[c].parent;
            tree[c].count++;
        }
    }

    //add to root
    tree[HUFFTREESIZE-1].count++;
}


//todo add sub_hufftree y buffer
void continuous_huffman_comp(char * source, char * dest){
    FILE * in = fopen(source, "rb"), * out = fopen(dest, "wb");

    //return if file is empty
    fseek(in, 0, SEEK_END);
    size_t fileSize;
    if ((fileSize = ftell(in)) == 0){
        fclose(in);
        fclose(out);
        return;
    }
    fseek(in, 0, SEEK_SET);

    //init hufftree
    huffTreeEntry tree[HUFFTREESIZE];
    init_hufftree(tree);

    //print bytes
    fprintf(out, "%lu", fileSize);

    //init stack
    char stack[256];
    int stackTop = 0;
    int byte = 0;
    int bit = 8;

    //compress and build tree as it goes
    int c = fgetc(in);;
    while (c != EOF){
        int aux = c;
        while (tree[aux].parent != NULLREF){
            stack[stackTop++] =  tree[aux].isRight;
            aux = tree[aux].parent;
        }

        //pack and print reverse bits
        while (stackTop > 0){
            byte |= stack[--stackTop] << --bit;
            if (!bit){
                fputc(byte, out);
                bit = 8;
                byte = 0;
            }
        }

        add_hufftree(tree, c);
        c = fgetc(in);
    }

    //print last byte
    if (bit != 8)
        fputc(byte, out);

    fclose(in);
    fclose(out);
}

void continuous_huffman_decomp(char * source, char * dest){
    huffTreeEntry tree[256 * 2 - 1];

    init_hufftree(tree);
}