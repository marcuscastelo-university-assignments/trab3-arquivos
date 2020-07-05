#pragma once

#include <stdio.h>
#include "bool.h"

#define H_INCREASE -1
#define H_DECREASE -2

typedef enum {
    BTHMASK_NONE = 0,
    BTHMASK_STATUS = 1,
    BTHMASK_NORAIZ = 2,
    BTHMASK_NRONIVEIS = 4,
    BTHMASK_PROXRRN = 8,
    BTHMASK_NROCHAVES = 16,
    BTHMASK_ALL = 31
} ChangedBTHeadersMask;

typedef struct _b_tree_header BTHeader;

BTHeader *b_tree_header_create(void);
void b_tree_header_delete(BTHeader **header_ptr);

void b_tree_header_write_to_bin(BTHeader *header, FILE *file);
void b_tree_header_read_from_bin(BTHeader *header, FILE *bin_file);

char b_tree_header_get_status(BTHeader *header);
void b_tree_header_set_status(BTHeader *header, char new_status);

int b_tree_header_get_noRaiz (BTHeader *header);
void b_tree_header_set_noRaiz (BTHeader *header, int new_value);

int b_tree_header_get_nroNiveis (BTHeader *header);
void b_tree_header_set_nroNiveis (BTHeader *header, int new_value);

int b_tree_header_get_proxRRN (BTHeader *header);
void b_tree_header_set_proxRRN (BTHeader *header, int new_value);

int b_tree_header_get_nroChaves (BTHeader *header);
void b_tree_header_set_nroChaves (BTHeader *header, int new_value);
