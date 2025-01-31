//
// Created by heros on 19/09/24.
//

#ifndef SHELL_SO_LISTS_H
#define SHELL_SO_LISTS_H

typedef struct list * list;

//mem management instructions
list list_init();
void list_free(list l);

//list management
void list_append(list l , void * element);
void * list_pop(list l);
void list_remove(list l, int pos);
void list_add(list l, int pos, void * element);
void list_ordered_insert(list l, void * element, int (*comparator)(const void *, const void *));

//element management
void * list_get(list l, int pos);
void list_set(list l, int pos, void * element);
int list_search(list l, void * element, int (*comparator)(const void *, const void *));
int list_bsearch(list l, void * element, int (*comparator)(const void *, const void *));
int list_length(list l);

#endif //SHELL_SO_LISTS_H
