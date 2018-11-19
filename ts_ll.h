#ifndef _TS_LL_
#define _TS_LL_

/* Typedef for thread safe linked list */
typedef struct ts_ll TSLinkedList;

/* Returns a pointer to a thread safe linked list */
TSLinkedList *tsll_create();
/* Returns 1/0 if item was inserted/ not inserted */
int tsll_insert(TSLinkedList *tsll, int item);
/* Returns the item removed from the list */
int *tsll_remove(TSLinkedList *tsll, int pos);
/* Prints the contents of the linked list */
void print_tsll(TSLinkedList *tsll);
/* Destroys a linked list object */
void tsll_destroy(TSLinkedList *tsll);
/* Set done to true */
void tsll_set_done(TSLinkedList *);

#endif