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

void b_tree_node_free (BTreeNode *node) {
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
            break;
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

void b_tree_node_insert_P (BTreeNode *node, int P, int position) {
    if (node == NULL)
        return;

    for (int i = B_TREE_ORDER-2; i >= position; i--) {
        node->P[i+1] = node->P[i]; 
    }

    node->P[position] = P;

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

    node->n--;

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

////////////
int b_tree_node_get_RRN_that_fits (BTreeNode *node, int key) {
    if (node == NULL)
        return -1;

    bool enteredBeforeNodeEnd = false;
    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        int C = b_tree_node_get_C(node, i);

        if (C == -1 || key < C) {
            nodeRRN = b_tree_node_get_P(node, i);
            enteredBeforeNodeEnd = true;
            break;
        }

        else if (key == C) {
            nodeRRN = -2;
            break;
        }  
    }

    //Se não tiver entrado ainda (já chegamos no fim do nó), força a entrada à direita
    if (!enteredBeforeNodeEnd)
        nodeRRN = b_tree_node_get_P(node, B_TREE_ORDER-1);

    return nodeRRN;
}

int insertion_sort_insert_in_array (int *arr, int size, int value) {
    for (int i = 0; i < size; i++) {
        if (value < arr[i]) {
            for (int j = size-2; j >= i; j--) {
                arr[j+1] = arr[j];
            }

            arr[i] = size;
            return i;
        }
    }

    return -1;
}

int min (int a, int b) {
    if (a < b) return a;
    return b;
}

/*

*/
BTreeNode *b_tree_node_split_one_to_two(BTreeNode *parent, int C, int Pr, int P) {
	BTreeNode *new = b_tree_node_create(b_tree_node_get_nivel(parent));

    int *newC = (int*) malloc (sizeof(int) * B_TREE_ORDER);
    int *newPr = (int*) malloc (sizeof(int) * B_TREE_ORDER);
    int *newP = (int*) malloc (sizeof(int) * B_TREE_ORDER+1);

    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        newC[i] = b_tree_node_get_C(parent);
        newPr[i] = b_tree_node_get_Pr(parent);
    }

    int position = insertion_sort_insert_in_array(newC, B_TREE_ORDER, C);
    for (int i = B_TREE_ORDER-2; i >= position; i--) {
        newPr[i+1] = newPr[i];
    }
    newPr[position] = Pr;

    for (int i = 0; i < B_TREE_ORDER; i++) {
        newP[i] = b_tree_node_get_P(parent, i);
    }
    for (int i = B_TREE_ORDER-1; i >= position+1; i++) {
        newP[i+1] = newP[i];
    }
    newP[position+1] = P;

    //inserting in old
    for (int i = 0; i < B_TREE_ORDER/2; i++) {
        b_tree_node_set_item(parent, newC[i], newPr[i], i);
    }
    for (int i = B_TREE_ORDER/2; i < B_TREE_ORDER-1; i++) {
        b_tree_node_remove_item(parent, B_TREE_ORDER/2);
    }

    //inserting in new
    for (int i = B_TREE_ORDER/2; i < B_TREE_ORDER; i++) {
        b_tree_node_sorted_insert_item(new, newC[i], newPr[i]);
    }

    return new; 
}

void b_tree_node_print (BTreeNode *node) {
    if (node == NULL) {
        printf("No node to print\n");
        return;
    }

    printf("Nivel: %d\n", b_tree_node_get_nivel(node));
    printf("N: %d\n", b_tree_node_get_n(node));
    printf("C:");
    for (int i = 0; i < B_TREE_ORDER-1; i++)
        printf("%d ", b_tree_node_get_C(node, i));
    printf("\nPr: ");
    for (int i = 0; i < B_TREE_ORDER-1; i++)
        printf("%d ", b_tree_node_get_Pr(node, i));
    printf("\nP: ");
    for (int i = 0; i < B_TREE_ORDER; i++)
        printf("%d ", b_tree_node_get_P(node, i));
    printf("\n");

    return;
}
