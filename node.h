#ifndef _NODE_H_
#define _NODE_H_

/* Typedef for node */
typedef struct node Node;

/* Returns a pointer to a node object if successfull, NULL otherwise */
Node *create_node(int data, Node *next);
/* Returns the data stored at that node */
int get_data(Node *n);
/* Returns a pointer to the next node */
Node *get_next(Node *n);
/* Set the data stored at that node */
void set_data(Node *n, int data);
/* Set the next node */
void set_next(Node *n, Node *node);
/* Print data stored at that node */
void print_node(Node *n);
/* Delete node object */
void node_delete(Node *n);

#endif