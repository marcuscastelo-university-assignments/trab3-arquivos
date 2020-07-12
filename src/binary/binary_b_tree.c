#include "binary_b_tree.h"
#include "binary_io.h"

#include "debug.h"

BTreeNode *binary_read_b_tree_node (FILE *file_ptr) {
    if (file_ptr == NULL)
        return NULL;

    int nivel = binary_read_int(file_ptr);
    // int n = binary_read_int(file_ptr); TODO: dar uso lucas
    int C, Pr, P;

    BTreeNode *node = b_tree_node_create(nivel);

    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        C = binary_read_int(file_ptr);
        Pr = binary_read_int(file_ptr);
        b_tree_node_sorted_insert_item(node, C, Pr);
    }

    for (int i = 0; i < B_TREE_ORDER; i++) {
        P = binary_read_int(file_ptr);
        b_tree_node_set_P(node, P, i);
    }

    if (node == NULL)
        return NULL;

    return node;
}

void binary_write_b_tree_node (FILE *file_ptr, BTreeNode *node) {
    if (file_ptr == NULL || node == NULL) {
        DP("ERROR: invalid parameters @binary_write_b_tree_node()");
        return;
    }

    binary_write_int(file_ptr, b_tree_node_get_nivel(node));
    binary_write_int(file_ptr, b_tree_node_get_n(node));

    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        binary_write_int(file_ptr, b_tree_node_get_C(node, i));
        binary_write_int(file_ptr, b_tree_node_get_Pr(node, i));
    }

    for (int i = 0; i < B_TREE_ORDER; i++) {
        binary_write_int(file_ptr, b_tree_node_get_P(node, i));      
    }

    return;
}   