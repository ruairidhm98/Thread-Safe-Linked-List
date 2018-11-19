#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "node.h"
#include "ts_ll.h"

#define NUM_THREADS 4

/* Thread safe singly linked list */
struct ts_ll {
    Node *head;
    Node *tail;
    int done;
    unsigned long size;
    pthread_mutex_t mutex;
    pthread_cond_t delete;
};

/* Aruments used in concurrent insert */
struct args {
    TSLinkedList *tsll;
    int item;
};

/* Arguments used in concurrent delete */
struct del_args {
    TSLinkedList *tsll;
    int pos;
};

/* Returns a pointer to a thread safe linked list if successfull, NULL otherwise */
TSLinkedList *tsll_create() {

    TSLinkedList *tsll;

    tsll = (TSLinkedList *) malloc(sizeof(TSLinkedList));
    /* Print error message and return NULL if memory allocation fails */
    if (!tsll) {
        fprintf(stderr, "Error: memory allocaiton failed\n");
        free((void *) tsll);
        tsll = NULL;
        return tsll;
    }
    tsll -> head = NULL;
    tsll -> tail = NULL;
    tsll -> size = 0L;
    tsll -> done = 0;
    /* Print error message, delete linked list object and return NULL if mutex fails to create */
    if (pthread_mutex_init(&tsll -> mutex, NULL)) {
        fprintf(stderr, "Error: mutex failed to create\n");
        pthread_mutex_destroy(&tsll -> mutex);
        free((void *) tsll);
        tsll = NULL;
    }
    /* Print error message, delete linked list object and return NULL if condition variable fails to create */
    if (pthread_cond_init(&tsll -> delete, NULL)) {
        fprintf(stderr, "Error: condition variable failed to create\n");
        pthread_mutex_destroy(&tsll -> mutex);
        pthread_cond_destroy(&tsll -> delete);
        free((void *) tsll);
        tsll = NULL;
    }

    return tsll;
}

/* Returns 1/0 is item was inserted/ not inserted */
int tsll_insert(TSLinkedList *tsll, int item) {

    Node *newNode, *cursor, *prev;

    newNode = create_node(item, NULL);
    /* If new node failed to create, return failure */
    if (!newNode) return 0;
    /* Critical region */
    pthread_mutex_lock(&tsll -> mutex);
    /* If the list is empty */
    if (!tsll -> head) {
        tsll -> head = newNode;
        tsll -> tail = newNode;
    }
    /* Insert at head */
    else if (item < get_data(tsll -> head)) {
        set_next(newNode, tsll -> head);
        tsll -> head = newNode;
    }
    /* Insert at the tail */
    else if (item > get_data(tsll -> tail)) {
        set_next(tsll -> tail, newNode);
        tsll -> tail = newNode;
    }
    /* Iterate to find point of insertion */
    else {
        cursor = tsll -> head;
        while (cursor && item > get_data(cursor)) {
            prev = cursor;
            cursor = get_next(cursor);
        }
        set_next(prev, newNode);
        set_next(newNode, cursor);
    }
    tsll -> size++;
    pthread_mutex_unlock(&tsll -> mutex);
    pthread_cond_signal(&tsll -> delete);

    return 1;
}

/* Returns the data stored at the node that was deleted */
int tsll_delete(TSLinkedList *tsll, int pos) {

    int result;
    Node *temp, *prev;

    pthread_mutex_lock(&tsll -> mutex);
    /* Wait until there is something to delete */
    while (!tsll -> size || !tsll -> done) 
        pthread_cond_wait(&tsll -> delete, &tsll -> mutex);
    
    /* We can still remove */
    if (tsll -> size) {
        if (pos >= tsll -> size) return -1;
        temp = tsll -> head;
        /* Iterate to find position */
        while (pos) {
            prev = temp;
            temp = get_next(temp);
            pos--;
        }
        if (pos != 0) return -1;
        set_next(prev, get_next(temp));
        result = get_data(temp);
        node_delete(temp);
    }
    else {
        pthread_mutex_unlock(&tsll -> mutex);
        return -1;
    }
    pthread_mutex_unlock(&tsll -> mutex);

    return result;
}

/* Prints a linked list object */
void print_tsll(TSLinkedList *tsll) {

    Node *cursor;

    if (!tsll || !tsll -> head) return;
    cursor = tsll -> head;
    while (cursor) {
        printf("%d ", get_data(cursor));
        if (get_next(cursor)) printf("-> ");
        cursor = get_next(cursor);
    }
    printf("\n");

}

/* Deletes a linked list object */
void tsll_destroy(TSLinkedList *tsll) {

    Node *cursor, *temp;

    cursor = tsll -> head;
    pthread_mutex_destroy(&tsll -> mutex);
    pthread_cond_destroy(&tsll -> delete);
    while (cursor) {
        temp = cursor;
        cursor = get_next(cursor);
        node_delete(temp);
    }   
    free((void *) tsll);

}

/* Set done to true */
void tsll_set_done(TSLinkedList *tsll) { tsll -> done = 1; }

/* Multi threaded function used to insert */
void *insert_parallel(void *arg) {
    
    TSLinkedList *tsll;
    struct args *args;
    int item;
    
    args = (struct args *) arg;
    item = args -> item;
    tsll = args -> tsll;
    tsll_insert(tsll, item);

    pthread_exit(NULL);
}

/* Multi threaded function used to remove */
void *delete_parallel(void *arg) {

    struct del_args *args;
    TSLinkedList *tsll;
    int pos;
    
    args = (struct del_args *) arg;
    tsll = args -> tsll;
    pos = args -> pos;

    tsll_delete(tsll, pos);
    
    pthread_exit(NULL);
}

int main() {

    pthread_t threads[NUM_THREADS], insertion[NUM_THREADS];
    struct del_args *delArgs;
    TSLinkedList *tsll;
    struct args **args;
    double t1, t2;
    int i, j;

    tsll = tsll_create();
    if (!tsll) return -1;
    print_tsll(tsll);
    /* Create arguments and then spawn threads */
    args = (struct args **) malloc(sizeof(struct args *) * NUM_THREADS);
    if (!args) {
        fprintf(stderr, "Error: memory allocation failed\n");
        tsll_destroy(tsll);
        free((void *) args);
        return -1;
    }
    for (i = 0; i < NUM_THREADS; i++) {
        args[i] = (struct args *) malloc(sizeof(struct args *));
        if (!args[i]) {
            for (j = 0; j < i; j++) free((void *) args[j]);
            free((void *) args);
            tsll_destroy(tsll);
            return -1;
        }
        args[i] -> tsll = tsll;
    }

    /* Create arguments for delete */
    delArgs = (struct del_args *) malloc(sizeof(struct del_args) * NUM_THREADS);
    if (!delArgs) {
        fprintf(stderr, "Error: memory allocation failed\n");
        for (i = 0; i < NUM_THREADS; i++) free((void *) args[i]);
        free((void *) args);
        tsll_destroy(tsll);
        return -1;
    }
    for (i = 0; i < NUM_THREADS; i++) {
        delArgs[i].pos = rand () % 5;
        delArgs[i].tsll = tsll;
    }
    t1 = clock();
    /* Spawn threads */
    for (i = 0; i < NUM_THREADS; i++) {
        args[i] -> item = i;
        if (pthread_create(&threads[i], NULL, insert_parallel, (void *) args[i])) {
            fprintf(stderr, "Error: failed to create thread %d\n", i+1);
            for (j = 0; j < NUM_THREADS; j++) free((void *) args[j]);
            free((void *) args);
            tsll_destroy(tsll);
            return -1;
        }
    }
    /* Wait for threads to finish */
    for (i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);
    print_tsll(tsll);

    for (i = 0; i < NUM_THREADS; i++) 
        if (pthread_create(&threads[i], NULL, delete_parallel, (void *) &delArgs[i])) {
            fprintf(stderr, "Error: failed to create %d\n", i+1);
            for (j = 0; j < NUM_THREADS; j++) free((void *) args[j]);
            free((void *) args);
            tsll_destroy(tsll);
            return -1;
        }

    /* Spawn threads */
    for (i = 0; i < NUM_THREADS; i++) {
        args[i] -> item = rand() % 25;
        if (pthread_create(&insertion[i], NULL, insert_parallel, (void *) args[i])) {
            fprintf(stderr, "Error: failed to create thread %d\n", i + 1);
            for (j = 0; j < NUM_THREADS; j++) free((void *) args[j]);
            free((void *) args);
            tsll_destroy(tsll);
            return -1; 
        }
    }
 
    for (i = 0; i < NUM_THREADS; i++) pthread_join(insertion[i], NULL);
    tsll_set_done(tsll);
    pthread_cond_broadcast(&tsll -> delete);
    for (i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);
    t2 = clock();
    print_tsll(tsll);

    /* Output time taken */
    printf("Program taken %.3fms\n", 1000 * ((t2-t1) / CLOCKS_PER_SEC));
    /* Delete heap memory used */
    tsll_destroy(tsll);
    for (i = 0; i < NUM_THREADS; i++) free((void *) args[i]);
    free((void *) args);
    (void) j;

    return -1;
}
