#ifndef LINKED_LIST_ADT_H
#define LINKED_LIST_ADT_H

#include "list.h"

// Alias for convenience - LinkedListADT is a pointer to List
typedef List* LinkedListADT;

// ADT-style wrappers for list operations
static inline LinkedListADT createLinkedListADT() {
    LinkedListADT list = (LinkedListADT)mm_alloc(sizeof(List));
    list_init(list);
    return list;
}

static inline void freeLinkedListADTDeep(LinkedListADT list) {
    if (!list) return;
    
    // Free all nodes (data was already freed by caller if needed)
    Node *current = list->head;
    while (current) {
        Node *next = current->next;
        mm_free(current);
        current = next;
    }
    mm_free(list);
}

static inline Node* appendElement(LinkedListADT list, void *data) {
    return list_append(list, data);
}

static inline Node* getFirst(LinkedListADT list) {
    return list_get_first(list);
}

static inline void removeNode(LinkedListADT list, Node *node) {
    list_remove(list, node);
}

static inline int isEmpty(LinkedListADT list) {
    return list_is_empty(list);
}

#endif // LINKED_LIST_ADT_H

