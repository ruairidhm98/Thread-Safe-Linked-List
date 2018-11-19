#include <stdio.h>
#include <stdlib.h>
#include "node.h"

/* Node structure */
struct node {
    int data; // data stored at node
    struct node *next; // pointer to next node
};

/* Returns a pointer to a node object if successfull, NULL otherwise */
Node *create_node(int data, Node *next) {

    Node *newNode;

    newNode = (Node *) malloc(sizeof(Node));
    /* Print error message and return NULL if memory allocation fails */
    if (!newNode) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free(newNode);
        newNode = NULL;
        return newNode;
    }
    newNode -> data = data;
    newNode -> next = next;

    return newNode;
}

/* Returns the data stored at that node */
int get_data(Node *node) { return node -> data; }

/* Returns the next node */
Node *get_next(Node *node) { return node -> next; }

/* Set the data stored at that node */
void set_data(Node *node, int data) { node -> data = data; }

/* Set the next node */
void set_next(Node *n, Node *node) { n -> next = node; }

/* Print data stored at that node */
void print_node(Node *n) { printf("%d\n", n -> data); }

/* Delete node object */
void node_delete(Node *n) { free((void *) n); }