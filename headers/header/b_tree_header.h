// #pragma once

// #include <stdio.h>
// #include "bool.h"

// #define H_INCREASE -1
// #define H_DECREASE -2

// typedef enum {
//     HMASK_NONE = 0,
//     HMASK_STATUS = 1,
//     HMASK_NEXTRRN = 2,
//     HMASK_REGISTRIESCOUNT = 4,
//     HMASK_REMOVEDCOUNT = 8,
//     HMASK_UPDATEDCOUNT = 16,
//     HMASK_ALL = 31
// } ChangedBTHeadersMask;

// typedef struct _header RegistryHeader;

// RegistryHeader *header_create(void);
// void header_delete(RegistryHeader **header_ptr);

// void header_read_from_bin(RegistryHeader *header, FILE *bin_file);
// void header_write_to_bin(RegistryHeader *header, FILE *bin_file);

// char header_get_status (RegistryHeader *header);
// void header_set_status (RegistryHeader *header, char new_status);

// int header_get_registries_count (RegistryHeader *header);
// void header_set_registries_count (RegistryHeader *header, int counter);
// bool has_registries_inserted (RegistryHeader *header);

// int header_get_removed_count (RegistryHeader *header);
// void header_set_removed_count (RegistryHeader *header, int counter);

// int header_get_updated_count (RegistryHeader *header);
// void header_set_updated_count (RegistryHeader *header, int counter);

// int header_get_next_RRN (RegistryHeader *header);
// void header_set_next_RRN (RegistryHeader *header, int new_rrn);