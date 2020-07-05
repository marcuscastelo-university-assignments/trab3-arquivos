#ifndef __REGISTRY_LINKED_LIST__H__
#define __REGISTRY_LINKED_LIST__H__

#include "registry.h"
#include "registry_array.h"
#include "bool.h"

typedef struct reg_linked_list_ RegistryLinkedList;

RegistryLinkedList *registry_linked_list_create();
void registry_linked_list_insert(RegistryLinkedList *list, VirtualRegistry *data);

VirtualRegistryArray *registry_linked_list_to_array(RegistryLinkedList *list);

int registry_linked_list_get_size(RegistryLinkedList *list);

void registry_linked_list_delete(RegistryLinkedList **list_ptr, bool should_delete_data);

#endif  //!__REGISTRY_LINKED_LIST__H__