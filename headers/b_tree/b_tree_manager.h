#ifndef __B_TREE_MANAGER__H__
#define __B_TREE_MANAGER__H__

#include <stdio.h>

#include "bool.h"
#include "b_tree_header.h"
#include "b_tree_node.h"
#include "open_mode.h"
#include "pair.h"


typedef struct _insert_answer insert_answer;
typedef struct _b_tree_manager BTreeManager;

BTreeManager *b_tree_manager_create(void);
bool b_tree_manager_open(BTreeManager *manager, char* bin_filename, OPEN_MODE mode);

void b_tree_manager_write_headers_to_disk(BTreeManager *manager);
void b_tree_manager_read_headers_from_disk(BTreeManager *manager);

void b_tree_manager_close(BTreeManager *manager);
void b_tree_manager_free(BTreeManager **manager_ptr);

void b_tree_manager_insert(BTreeManager *manager, int key, int value);
pairIntInt b_tree_manager_search_for (BTreeManager *manager, int key);


BTreeHeader *b_tree_manager_get_headers(BTreeManager *man);

#endif  //!__B_TREE_MANAGER__H__