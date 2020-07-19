#ifndef __REGISTRY_HEADER__H__
#define __REGISTRY_HEADER__H__

#include <stdio.h>
#include "bool.h"

#define H_INCREASE -1
#define H_DECREASE -2

//Definição de valores para uso mascara de bits em registry_header.c
typedef enum {
    RHMASK_NONE = 0,
    RHMASK_STATUS = 1,
    RHMASK_NEXTRRN = 2,
    RHMASK_REGISTRIESCOUNT = 4,
    RHMASK_REMOVEDCOUNT = 8,
    RHMASK_UPDATEDCOUNT = 16,
    RHMASK_ALL = 31
} ChangedRHeadersMask;

typedef struct _reg_header RegistryHeader;

RegistryHeader *reg_header_create(void);
void reg_header_delete(RegistryHeader **header_ptr);

void reg_header_read_from_bin(RegistryHeader *header, FILE *bin_file);
void reg_header_write_to_bin(RegistryHeader *header, FILE *bin_file);

char reg_header_get_status (RegistryHeader *header);
void reg_header_set_status (RegistryHeader *header, char new_status);

int reg_header_get_registries_count (RegistryHeader *header);
void reg_header_set_registries_count (RegistryHeader *header, int counter);
bool has_registries_inserted (RegistryHeader *header);

int reg_header_get_removed_count (RegistryHeader *header);
void reg_header_set_removed_count (RegistryHeader *header, int counter);

int reg_header_get_updated_count (RegistryHeader *header);
void reg_header_set_updated_count (RegistryHeader *header, int counter);

int reg_header_get_next_RRN (RegistryHeader *header);
void reg_header_set_next_RRN (RegistryHeader *header, int new_rrn);

#endif  //!__REGISTRY_HEADER__H__