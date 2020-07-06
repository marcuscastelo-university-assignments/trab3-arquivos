#ifndef __B_TREE_NODE__H__
#define __B_TREE_NODE__H__

#include "bool.h"

#define B_TREE_ORDER 6

typedef struct _b_tree_node BTreeNode;

BTreeNode* b_tree_node_create (int nivel);
void b_tree_node_delete (BTreeNode *node);

bool b_tree_node_sorted_insert_item (BTreeNode *node, int C, int Pr);
void b_tree_node_set_item (BTreeNode *node, int C, int Pr, int position);
void b_tree_node_push_back_P (BTreeNode *node, int P);
void b_tree_node_set_P (BTreeNode *node, int P, int position);
void b_tree_node_set_nivel (BTreeNode *node, int nivel);

int b_tree_node_get_n (BTreeNode *node);
int b_tree_node_get_nivel (BTreeNode *node);
int b_tree_node_get_C (BTreeNode *node, int position);
int b_tree_node_get_Pr (BTreeNode *node, int position);
int b_tree_node_get_P (BTreeNode *node, int position);

void b_tree_node_remove_item (BTreeNode *node, int position);
void b_tree_node_remove_P (BTreeNode *node, int position);

#endif  //!__B_TREE_NODE__H__