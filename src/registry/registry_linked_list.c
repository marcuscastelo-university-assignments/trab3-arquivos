#include "registry_linked_list.h"
#include "registry_array.h"

#include <stdlib.h>
#include "debug.h"

//Nó da lista ligada, contém os dados do registro e o próximo nó (NULL caso fim da lista)
typedef struct reg_ll_node
{
    VirtualRegistry *data;
    struct reg_ll_node *next;
} RegistryLinkedListNode;

//RegistryHeader da lista ligada, contendo o primeiro, último nó e o tamanho da lista
struct reg_linked_list_ {
    int size;
    RegistryLinkedListNode *first_node, *last_node;
};

/**
 *  Cria uma lista ligada
 *  Retorno:
 *      RegistryLinkedList * -> pointer para a lista criada 
 */
RegistryLinkedList *registry_linked_list_create() {
    RegistryLinkedList *list = malloc(sizeof(RegistryLinkedList));
    list->size = 0;
    list->first_node = list->last_node = NULL;
    return list;
}

/**
 *  Função auxiliar que cria nós da lista ligada com valores padrão
 *  Parâmetros:
 *      VirtualRegistry *data -> dados do registro do nó
 *  Retorno:
 *      RegistryLinkedListNode -> nó ainda não conectado na lista, com os dados informados
 */
static RegistryLinkedListNode *_create_node(VirtualRegistry *data) {
    RegistryLinkedListNode *node = malloc(sizeof(RegistryLinkedListNode));
    node -> data = data;
    node -> next = NULL;
    return node;
}

/**
 *  Insere um VirtualRegistry no fim da lista ligada
 *  Parâmetros:
 *      RegistryLinkedList *linked_list -> lista na qual se realizará a inserção
 *      VirtualRegistry *data -> registro a ser inserido
 */
void registry_linked_list_insert(RegistryLinkedList *linked_list, VirtualRegistry *data) {
    //Se vazia, insira no começo
    if (linked_list->size == 0) {
        linked_list->first_node = linked_list->last_node = _create_node(data);
        linked_list->size = 1;
        return;
    }

    //Se não vazia, insira após o último
    linked_list -> last_node -> next = _create_node(data);
    linked_list -> last_node = linked_list -> last_node -> next;
    linked_list -> size++;
}

/**
 *  Transforma uma lista ligada em um array VirtualRegistry**
 *  Parâmetros:
 *      RegistryLinkedList *list -> lista a ser transformada
 *  Retorno:
 *      VirtualRegistryArray* -> vetor gerado
 */ 
VirtualRegistryArray *registry_linked_list_to_array(RegistryLinkedList *list) {
    //Validação de parâmetros
    if (list == NULL) {
        DP("ERROR: invalid null list parameter @registry_linked_list_to_array()\n");
        return NULL;
    }

    //Tenta criar o array que armazenará a lista
    VirtualRegistry **reg_data_arr = malloc(sizeof(VirtualRegistry*) * list->size);
    if (reg_data_arr == NULL) {
        DP("ERROR: Failed to allocate %d VirtualRegistries @registry_linked_list_to_array()\n", list->size);
        return NULL;
    }  
    
    //Insere cada nó no array
    RegistryLinkedListNode *node = list->first_node;
    for (int i = 0; node != NULL; i++) {
        reg_data_arr[i] = node->data;
        node = node->next;
    }
    return virtual_registry_array_create(reg_data_arr, list->size);

}

//Obtém o tamanho da lista
int registry_linked_list_get_size(RegistryLinkedList *list) {
    return list->size;
}

/**
 *  Função recursiva que apaga todos os nós de uma lista ligada
 *  Parâmetros:
 *      RegistryLinkedListNode *node -> nó inicial a ser deletado
 *      bool (typedef char) should_delete_data -> flag se a função deve ou não dar free nos VirtualRegistry* dentro da lista
 *  Retorno: void
 */
static void _recursive_delete_nodes(RegistryLinkedListNode *node, bool should_delete_data) {
    if (node == NULL) return;
    _recursive_delete_nodes(node->next, should_delete_data);
    
    if (should_delete_data) virtual_registry_free(&node->data);
    node->data = NULL;
    node->next = NULL;
    free(node);
}

/**
 *  Apaga toda a memória usada pela lista ligada, exceto os VirtualRegistry*.
 *  Estes devem ser mantidos pois o programa o utiliza após a deleção da lista.
 *  Parâmetros:
 *      RegistryLinkedList **list_ptr -> referência à variável que aponta para a lista ligada
 *      bool (typedef char) should_delete_data -> flag se a função deve ou não dar free nos VirtualRegistry* dentro da lista
 *  Retorno: void
 */
void registry_linked_list_delete(RegistryLinkedList **list_ptr, bool should_delete_data) {
    if (list_ptr == NULL) return;

    #define list (*list_ptr)

    //Apaga todos os nós com a função recursiva
    _recursive_delete_nodes(list->first_node, should_delete_data);

    //Apaga dados do header da lista
    list->first_node = NULL;
    list->last_node = NULL;
    list->size = 0;

    //Apaga a struct do header
    free(list);

    //Define como NULL para evitar futuros acidentes com o pointer
    list = NULL;

    #undef list
}
