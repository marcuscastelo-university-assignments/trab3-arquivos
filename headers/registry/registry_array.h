#ifndef __REGISTRY_ARRAY__H__
#define __REGISTRY_ARRAY__H__

#include "registry.h"

/*
    Struct auxiliar para retornar vetores com seu tamanho de maneira legível
*/
typedef struct {
    VirtualRegistry **data_arr;
    int size;
} VirtualRegistryArray;

VirtualRegistryArray *virtual_registry_array_create_unique(VirtualRegistry *registry);
VirtualRegistryArray *virtual_registry_array_create(VirtualRegistry **array, int size);
void virtual_registry_array_delete(VirtualRegistryArray **array_ptr);
bool virtual_registry_array_contains(VirtualRegistryArray *search_terms_array, VirtualRegistry *reg_data, bool (*compare_func)(VirtualRegistry* reg_data1, VirtualRegistry* reg_data2));

#endif  //!__REGISTRY_ARRAY__H__