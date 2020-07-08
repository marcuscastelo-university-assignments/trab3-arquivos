#include "linked_list_int.h"

#include <stdio.h>
#include <stdlib.h>

#include "bool.h"
#include "debug.h"

//Nó da lista ligada, contém os dados do registro e o próximo nó (NULL caso fim da lista)
struct _llprim_node_int
{
    T data;
    LinkedListIntNode *next;
};
//Header da lista ligada, contendo o primeiro, último nó e o tamanho da lista
struct _linked_list_primitive_int {
    int size;
    LinkedListIntNode *first_node, *last_node;
};

/**
 *  Cria uma lista ligada
 *  Retorno:
 *      LinkedListInt * -> pointer para a lista criada 
 */
LinkedListInt *_linked_list_create() {
    LinkedListInt *list = malloc(sizeof(LinkedListInt));
    list->size = 0;
    list->first_node = list->last_node = NULL;
    return list;
}

/**
 *  Função auxiliar que cria nós da lista ligada com valores padrão
 *  Parâmetros:
 *      Virtual *data -> dados do registro do nó
 *  Retorno:
 *      LinkedListIntNode -> nó ainda não conectado na lista, com os dados informados
 */
static LinkedListIntNode *_create_node(T data) {
    LinkedListIntNode *node = malloc(sizeof(LinkedListIntNode));
    node -> data = data;
    node -> next = NULL;
    return node;
}

/**
 *  Insere um Virtual no fim da lista ligada
 *  Parâmetros:
 *      LinkedListInt *linked_list -> lista na qual se realizará a inserção
 *      Virtual *data -> registro a ser inserido
 */
void _linked_list_insert(LinkedListInt *linked_list, T data) {
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
 *  Transforma uma lista ligada em um array Virtual**
 *  Parâmetros:
 *      LinkedListInt *list -> lista a ser transformada
 *  Retorno:
 *      VirtualArray* -> vetor gerado
 */ 
TArr _linked_list_to_array(LinkedListInt *list) {
    //Validação de parâmetros
    if (list == NULL) {
        DP("ERROR: invalid null list parameter @_linked_list_to_array()\n");
        return (TArr){NULL,0};
    }

    //Tenta criar o array que armazenará a lista
    T *array = malloc(sizeof(T) * list->size);
    if (array == NULL) {
        DP("ERROR: Failed to allocate %d VirtualRegistries @_linked_list_to_array()\n", list->size);
        return (TArr){NULL,0};
    }  
    
    //Insere cada nó no array
    LinkedListIntNode *node = list->first_node;
    for (int i = 0; node != NULL; i++) {
        array[i] = node->data;
        node = node->next;
    }
    return (TArr){array, list->size};

}

//Obtém o tamanho da lista
int _linked_list_get_size(LinkedListInt *list) {
    return list->size;
}

/**
 *  Função recursiva que apaga todos os nós de uma lista ligada
 *  Parâmetros:
 *      LinkedListIntNode *node -> nó inicial a ser deletado
 *      bool (typedef char) should_delete_data -> flag se a função deve ou não dar free nos Virtual* dentro da lista
 *  Retorno: void
 */
static void _recursive_delete_nodes(LinkedListIntNode *node, bool should_delete_data) {
    if (node == NULL) return;
    _recursive_delete_nodes(node->next, should_delete_data);

    // if (should_delete_data) virtual__delete(&node->data); TODO: criar a funçao
    node->data = DEFAULT_T_VALUE;
    node->next = NULL;
    free(node);
}

/**
 *  Apaga toda a memória usada pela lista ligada, exceto os Virtual*.
 *  Estes devem ser mantidos pois o programa o utiliza após a deleção da lista.
 *  Parâmetros:
 *      LinkedListInt **list_ptr -> referência à variável que aponta para a lista ligada
 *      bool (typedef char) should_delete_data -> flag se a função deve ou não dar free nos Virtual* dentro da lista
 *  Retorno: void
 */
void _linked_list_delete(LinkedListInt **list_ptr, bool should_delete_data) {
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


LinkedListInt *_linked_list_create();
void _linked_list_insert(LinkedListInt *list, T value);
void _linked_list_delete(LinkedListInt **list_ptr, bool should_delete_data);

TArr _linked_list_to_array(LinkedListInt *list);

int _linked_list_get_size(LinkedListInt *list);
