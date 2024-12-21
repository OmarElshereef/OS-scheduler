#include "headers.h"
typedef struct Priority_Node {
    int pid;
    struct Priority_Node* next;
    int priority;
    int memory_size;
    bool first_run;
} Priority_Node;

typedef struct {
    Priority_Node* head;
} SJF_Queue;

bool Add_Process_SJF(SJF_Queue* q, Priority_Node* node) {
    if (!q || !node) return false;

    if (q->head == NULL ) {
        // Insert at the head
        node->next = q->head;
        q->head = node;
        return true;
    }

    // Find the appropriate position
    Priority_Node* previous = q->head;
    Priority_Node* temp = previous->next;

    while (temp != NULL && temp->priority <= node->priority) {
        previous = temp;
        temp = temp->next;
    }

    // Insert in the list
    previous->next = node;
    node->next = temp;
    return true;
}
bool Remove_SJF(SJF_Queue* q) {
    if (!q || q->head == NULL) return false;

    Priority_Node* temp = q->head;
    q->head = temp->next;
    free(temp);
    return true;
}