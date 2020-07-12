#ifndef __REGISTRY__H__
#define __REGISTRY__H__

#include <stdio.h>
#include <bool.h>

#include "registry_mask.h"

#define DEFAULT_IDNASC -1
#define DEFAULT_IDADEMAE -1
#define DEFAULT_SEXOBEBE '0'

#define REGISTRY_SIZE 128

/*
    Struct para organizar as informações do registro. Como a struct é basicamente um acesso simples aos dados,
    sem muito processamento, julgou-se mais vantajoso tornar o acesso direto possível, tornando desnecessária a
    criação de um TAD com encapsulamento.
*/
struct _virtual_registry {
    RegistryFieldsMask fieldMask;
    char *cidadeMae;
    char *cidadeBebe;
    int idNascimento;
    int idadeMae;
    char *dataNascimento;
    char sexoBebe;
    char *estadoMae;
    char *estadoBebe;
};

typedef struct _virtual_registry VirtualRegistry;
typedef struct _virtual_registry VirtualRegistryFilter;
typedef struct _virtual_registry VirtualRegistryUpdater;

VirtualRegistry *virtual_registry_create_copy(VirtualRegistry *base);
VirtualRegistry *virtual_registry_create();
VirtualRegistry *virtual_registry_create_masked(RegistryFieldsMask compareFields);

void virtual_registry_free(VirtualRegistry **reg_data_ptr);

void virtual_registry_set_fieldmask(VirtualRegistry *reg_data, RegistryFieldsMask mask);
void virtual_registry_add_fieldmask(VirtualRegistry *reg_data, RegistryFieldsMask mask);
void virtual_registry_remove_fieldmask(VirtualRegistry *reg_data, RegistryFieldsMask mask);
RegistryFieldsMask virtual_registry_get_fieldmask(VirtualRegistry *reg_data);

void virtual_registry_set_field(VirtualRegistry *reg_data, char *campo, char *valor);

void virtual_registry_print (VirtualRegistry *reg_data);
void virtual_registry_print_all_fields (VirtualRegistry *reg_data);

bool virtual_registry_compare(VirtualRegistry *reg_data_1, VirtualRegistry *reg_data_2);


char *virtual_registry_read_value_from_input(char *field_name);
VirtualRegistry *virtual_registry_create_from_input(bool is_real_register);

#endif  //!__REGISTRY__H__