#pragma once

typedef int T;

typedef struct _linked_list_primitive_int LinkedListInt;
typedef struct _llprim_node_int LinkedListIntNode;

typedef struct {
    T *array;
    int size;
} TArr;

LinkedListInt *linked_list_create();
void linked_list_delete(LinkedListInt **list_ptr);

void linked_list_insert(LinkedListInt *list, T value);

LinkedListIntNode *linked_list_int_get(LinkedListInt *list, int index);

int registry_linked_list_get_size(LinkedListInt *list);
