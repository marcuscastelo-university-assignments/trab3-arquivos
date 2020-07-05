#ifndef __BINARY_B_TREE__H__
#define __BINARY_B_TREE__H__

#include <stdio.h>
#include "b_tree_node.h"

BTreeNode* binary_read_b_tree_node(FILE *file_ptr);
void binary_write_b_tree_node(FILE *file_ptr, BTreeNode *node);

#endif  //!__BINARY_B_TREE__H__