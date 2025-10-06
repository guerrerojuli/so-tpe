#ifndef LIST_H
#define LIST_H

// Doubly-linked list ADT for scheduler queues

typedef struct Node {
    void *data;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct List {
    Node *head;
    Node *tail;
    Node *iterator;  // For iteration
    int size;
} List;

// List operations
void list_init(List *list);
Node *list_append(List *list, void *data);
Node *list_prepend(List *list, void *data);
void *list_remove(List *list, Node *node);  // Frees node, not data
Node *list_get_first(List *list);
int list_is_empty(List *list);

// Iterator
void list_begin(List *list);
int list_has_next(List *list);
void *list_next(List *list);

#endif // LIST_H
