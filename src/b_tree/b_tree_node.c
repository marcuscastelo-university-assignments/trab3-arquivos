#include "b_tree_node.h"
#include <stdlib.h>

struct _b_tree_node {
    int nivel;
    int n;
    int C[B_TREE_ORDER-1];
    int Pr[B_TREE_ORDER-1];
    int P[B_TREE_ORDER];
};

BTreeNode* b_tree_node_create (int nivel) {
    BTreeNode *bTreeNode = (BTreeNode*) malloc (sizeof(BTreeNode));
    if (bTreeNode == NULL)
        return NULL;

    bTreeNode->nivel = nivel;
    bTreeNode->n = 0;

    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        bTreeNode->C[i] = -1;
        bTreeNode->Pr[i] = -1;
    }

    for (int i = 0; i < B_TREE_ORDER; i++) {
        bTreeNode->P[i] = -1;
    }

    return bTreeNode; 
}

void b_tree_node_add_item (BTreeNode *node, int C, int Pr) {
    if (node == NULL)
        return;
    
    int n = node->n;
    node->C[n] = C;
    node->Pr[n] = Pr;
    node->n = ++n;

    return;
}

void b_tree_node_push_back_P (BTreeNode *node, int P) {
    if (node == NULL)
        return;

    for (int i = 0; i < B_TREE_ORDER; i++) {
        if (node->P[i] == -1) {
            node->P[i] = P;
            break;
        }
    }

    return;
}

void b_tree_node_set_P (BTreeNode *node, int P, int position) {
    if (node == NULL)
        return;

    node->P[position] = P;

    return;
}

int b_tree_node_get_nivel (BTreeNode *node) {
    if (node == NULL)
        return -1;

    return node->nivel;
}
 
int b_tree_node_get_n (BTreeNode *node) {
    if (node == NULL)
        return -1;

    return node->n;
}

int b_tree_node_get_C (BTreeNode *node, int position) {
    if (node == NULL)
        return -1;

    if (position >= B_TREE_ORDER-1 || position < 0)
        return -1;

    return node->C[position];
}

int b_tree_node_get_Pr (BTreeNode *node, int position) {
    if (node == NULL)
        return -1;

    if (position >= B_TREE_ORDER-1 || position < 0)
        return -1;

    return node->Pr[position];
}

int b_tree_node_get_P (BTreeNode *node, int position) {
    if (node == NULL)
        return -1;

    if (position >= B_TREE_ORDER || position < 0)
        return -1;    
    
    return node->P[position];
}

