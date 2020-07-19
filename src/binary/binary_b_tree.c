#include "binary_b_tree.h"
#include "binary_io.h"

#include "debug.h"


/*
    Le um node de arvore-B no disco a partir da posicao atual do cursor
    Parametros:
        file_ptr -> o ponteiro do arquivo para ler
    Retorno:
        BTreeNode*. o node que foi lido
*/
BTreeNode *binary_read_b_tree_node (FILE *file_ptr) {
    if (file_ptr == NULL)
        return NULL;

    int nivel = binary_read_int(file_ptr);
    binary_read_int(file_ptr);
    int C, Pr, P;

    //cria o node
    BTreeNode *node = b_tree_node_create(nivel);

    if (node == NULL)
        return NULL;

    //le os itens (C e Pr)
    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        C = binary_read_int(file_ptr);
        Pr = binary_read_int(file_ptr);
        b_tree_node_sorted_insert_item(node, C, Pr);
    }

    //le os P's
    for (int i = 0; i < B_TREE_ORDER; i++) {
        P = binary_read_int(file_ptr);
        b_tree_node_set_P(node, P, i);
    }

    return node;
}

/*
    Escreve um node no disco na posicao atual do cursor
    Parametros:
        file_ptr -> o ponteiro do arquivo onde sera' escrito
        node -> o node que sera escrito
*/
void binary_write_b_tree_node (FILE *file_ptr, BTreeNode *node) {
    if (file_ptr == NULL || node == NULL) {
        DP("ERROR: invalid parameters @binary_write_b_tree_node()");
        return;
    }

    //escreve o nivel
    binary_write_int(file_ptr, b_tree_node_get_nivel(node));
    //escreve o N
    binary_write_int(file_ptr, b_tree_node_get_n(node));
    
    //escreve os itens
    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        binary_write_int(file_ptr, b_tree_node_get_C(node, i));
        binary_write_int(file_ptr, b_tree_node_get_Pr(node, i));
    }

    //escreve os P's
    for (int i = 0; i < B_TREE_ORDER; i++) {
        binary_write_int(file_ptr, b_tree_node_get_P(node, i));      
    }

    return;
}   