//
// Created by heros on 31/01/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lists.h"

int comp(void * a, void * b){
   return *(int *)a - *(int *)b;
}

int main(){
   srand(time(NULL));
   list l = list_init();

   for (int i = 0; i < 10; i++){
      char * r = malloc(sizeof(int));
      r[0] = ' ' + (rand() % 64) + 1;
      r[1] = '\0';
      list_ordered_insert(l, r, strcmp);
   }

   char h[] = "}";
   list_ordered_insert(l, h, strcmp);

   char c[] = "{";
   list_ordered_insert(l, c, strcmp);

   int pos = list_bsearch(l, c, strcmp);

   for (int i = 0; i < 12; i++){
      printf("%s\n", (char *)list_get(l, i));
   }

   printf("on pos %i: %s\n", pos, c);

   return 0;
}