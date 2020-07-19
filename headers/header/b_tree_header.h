#ifndef __B_TREE_HEADER__H__
#define __B_TREE_HEADER__H__

#include <stdio.h>
#include "bool.h"

#define H_INCREASE -1
#define H_DECREASE -2

//Definição de valores para uso mascara de bits em b_tree_header.c
typedef enum {
    BTHMASK_NONE = 0,
    BTHMASK_STATUS = 1,
    BTHMASK_NORAIZ = 2,
    BTHMASK_NRONIVEIS = 4,
    BTHMASK_PROXRRN = 8,
    BTHMASK_NROCHAVES = 16,
    BTHMASK_ALL = 31
} ChangedBTHeadersMask;

typedef struct _b_tree_header BTreeHeader;

BTreeHeader *b_tree_header_create(void);
void b_tree_header_free(BTreeHeader **header_ptr);

void b_tree_header_write_to_bin(BTreeHeader *header, FILE *file);
void b_tree_header_read_from_bin(BTreeHeader *header, FILE *bin_file);

char b_tree_header_get_status(BTreeHeader *header);
void b_tree_header_set_status(BTreeHeader *header, char new_status);

int b_tree_header_get_noRaiz (BTreeHeader *header);
void b_tree_header_set_noRaiz (BTreeHeader *header, int new_value);

int b_tree_header_get_nroNiveis (BTreeHeader *header);
void b_tree_header_set_nroNiveis (BTreeHeader *header, int new_value);

int b_tree_header_get_proxRRN (BTreeHeader *header);
void b_tree_header_set_proxRRN (BTreeHeader *header, int new_value);

int b_tree_header_get_nroChaves (BTreeHeader *header);
void b_tree_header_set_nroChaves (BTreeHeader *header, int new_value);

#endif  //!__B_TREE_HEADER__H__