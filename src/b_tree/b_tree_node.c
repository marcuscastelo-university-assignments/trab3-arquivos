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

    b_tree_node_set_nivel(bTreeNode, nivel);
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

void b_tree_node_f (BTreeNode *node) {
    if (node == NULL)
        return;

    free(node);

    return;
}

//////SET FUNCTIONS//////
void b_tree_node_set_nivel (BTreeNode *node, int nivel) {
    if (node == NULL)
        return;

    node->nivel = nivel;

    return;
}

bool b_tree_node_sorted_insert_item (BTreeNode *node, int C, int Pr) {
    if (node == NULL)
        return false;
    
    if (node->n == B_TREE_ORDER-1)
        return false;

    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        if (node->C[i] == -1) {
            node->C[i] = C;
            node->Pr[i] = Pr;
        }

        if (node->C[i] > C) {
            for (int j = B_TREE_ORDER-3; j >= i; j--) {
                node->C[j+1] = node->C[j];
                node->Pr[j+1] = node->Pr[j];
            }

            node->C[i] = C;
            node->Pr[i] = Pr;
        }
    }

    node->n++;

    return true;
}

void b_tree_node_set_item (BTreeNode *node, int C, int Pr, int position) {
    if (node == NULL)
        return;

    if (position >= B_TREE_ORDER-1 || position < 0)
        return;

    node->C[position] = C;
    node->Pr[position] = Pr;
    node->n++;

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

    if (position >= B_TREE_ORDER || position < 0)
        return;

    node->P[position] = P;

    return;
}

//////GET FUNCTIONS//////
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

//////REM FUNCTIONS//////
void b_tree_node_remove_item (BTreeNode *node, int position) {
    if (node == NULL)
        return;

    if (position >= B_TREE_ORDER-1 || position < 0)
        return;

    for (int i = position; i < B_TREE_ORDER-1-1; i++) {
        node->C[i] = node->C[i+1];
        node->Pr[i] = node->Pr[i+1];
    }

    node->C[B_TREE_ORDER-2] = -1;
    node->Pr[B_TREE_ORDER-2] = -1;

    return;
}

void b_tree_node_remove_P (BTreeNode *node, int position) {
    if (node == NULL)
        return;

    if (position >= B_TREE_ORDER || position < 0)
        return;

    for (int i = position; i < B_TREE_ORDER-1; i++) {
        node->P[i] = node->P[i+1];
    }

    node->P[B_TREE_ORDER-1] = -1;

    return;
}