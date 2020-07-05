#ifndef __B_TREE_MANAGER__H__
#define __B_TREE_MANAGER__H__

#include <stdio.h>

#include "bool.h"
#include "b_tree_header.h"
#include "b_tree_node.h"

#define INCREASE -1
#define DECREASE -2

#define FRONT 1
#define BACK -1

typedef struct _b_tree_manager BTreeManager;

BTreeManager *b_tree_manager_create(FILE *bin_file, BTHeader *header);
void b_tree_manager_delete(BTreeManager **manager_ptr);

void b_tree_manager_seek(BTreeManager *manager, int RRN);
void b_tree_manager_seek_first(BTreeManager *manager);
void b_tree_manager_seek_end(BTreeManager *manager);

BTreeNode *b_tree_manager_read_current(BTreeManager *manager);
BTreeNode *b_tree_manager_read_at(BTreeManager *manager, int RRN);

void b_tree_manager_write_current(BTreeManager *manager, BTreeNode *node);
void b_tree_manager_write_at(BTreeManager *manager, int RRN, BTreeNode *node);

void b_tree_manager_write_current_field_by_num(BTreeManager *manager, BTreeNode *node, int field_num);

void b_tree_manager_update_current(BTreeManager *manager, BTreeNode *node);
void b_tree_manager_update_at(BTreeManager *manager, int RRN, BTreeNode *node);

void b_tree_manager_delete_current (BTreeManager *manager);
void b_tree_manager_delete_at (BTreeManager *manager, int RRN);
bool b_tree_manager_is_current_deleted (BTreeManager *manager);

void b_tree_manager_jump_node (BTreeManager *manager, int direction);

#endif  //!__B_TREE_MANAGER__H__