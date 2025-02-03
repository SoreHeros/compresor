//
// Created by heros on 31/01/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dictionary.h"
#include "lists.h"

int main(){
   FILE * f = fopen("../dictionary.c", "r");
   FILE * out = fopen("dictionary.c.dict", "w");
   dict_comp(f, out);

   return 0;
}