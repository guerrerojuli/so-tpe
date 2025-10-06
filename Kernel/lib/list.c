// Doubly-linked list implementation
// Uses kernel's first-fit allocator for node allocation

#include "../include/list.h"
#include "../include/memoryManager.h"
#include <stddef.h>

extern KHEAPLCAB kernel_heap;

void list_init(List *list) {
    list->head = NULL;
    list->tail = NULL;
    list->iterator = NULL;
    list->size = 0;
}

Node *list_append(List *list, void *data) {
    Node *node = (Node *)k_heapLCABAlloc(&kernel_heap, sizeof(Node));

    node->data = data;
    node->next = NULL;
    node->prev = list->tail;

    if (list->tail)
        list->tail->next = node;
    list->tail = node;

    if (!list->head)
        list->head = node;

    list->size++;
    return node;
}

Node *list_prepend(List *list, void *data) {
    Node *node = (Node *)k_heapLCABAlloc(&kernel_heap, sizeof(Node));

    node->data = data;
    node->next = list->head;
    node->prev = NULL;

    if (list->head)
        list->head->prev = node;
    list->head = node;

    if (!list->tail)
        list->tail = node;

    list->size++;
    return node;
}

void *list_remove(List *list, Node *node) {
    if (!node) return NULL;

    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;

    list->size--;

    void *data = node->data;
    k_heapLCABFree(&kernel_heap, node);

    return data;
}

Node *list_get_first(List *list) {
    return list->head;
}

int list_is_empty(List *list) {
    return list->head == NULL;
}

void list_begin(List *list) {
    list->iterator = list->head;
}

int list_has_next(List *list) {
    return list->iterator != NULL;
}

void *list_next(List *list) {
    if (!list->iterator) return NULL;

    void *data = list->iterator->data;
    list->iterator = list->iterator->next;

    return data;
}
