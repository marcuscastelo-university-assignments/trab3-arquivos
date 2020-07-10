#ifndef __B_TREE_MANAGER__H__
#define __B_TREE_MANAGER__H__

#include <stdio.h>

#include "bool.h"
#include "b_tree_header.h"
#include "b_tree_node.h"

typedef struct _insert_answer insert_answer;
typedef struct _b_tree_manager BTreeManager;

BTreeManager *b_tree_manager_create(FILE *bin_file, BTHeader *header);
void b_tree_manager_delete(BTreeManager **manager_ptr);

void b_tree_manager_seek(BTreeManager *manager, int RRN);
void b_tree_manager_seek_first(BTreeManager *manager);

BTreeNode *b_tree_manager_read_current(BTreeManager *manager);
BTreeNode *b_tree_manager_read_at(BTreeManager *manager, int RRN);

void b_tree_manager_write_current(BTreeManager *manager, BTreeNode *node);
void b_tree_manager_write_at(BTreeManager *manager, int RRN, BTreeNode *node);

void b_tree_manager_insert(BTreeManager *manager, int idNascimento, int RRN);
int b_tree_manager_search_for (BTreeManager *manager, int idNascimento);

#endif  //!__B_TREE_MANAGER__H__