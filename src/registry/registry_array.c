#include "registry.h"
#include "registry_array.h"

#include <stdlib.h>
#include "debug.h"

VirtualRegistryArray *virtual_registry_array_create_unique(VirtualRegistry *registry) {
    VirtualRegistry **arr = malloc(1 * sizeof(VirtualRegistry*));
    arr[0] = registry;
    return virtual_registry_array_create(arr, 1);
}

/*
    Função para criar uma struct que representa um array de registros.
    Permite fácil acesso do tamanho do array informado.
    OBS: não faz cópia dos elementos do array informado, apenas o referencia.
    Parâmetros:
        array -> o array de registros
        size -> o tamanho desse array
    Retorno:
        VirtualRegistryArray* -> A struct que representa esse array
*/
VirtualRegistryArray *virtual_registry_array_create(VirtualRegistry **array, int size) {
    //Validação de parâmetros
    if (size < 0) {
        DP("ERROR: (parameter) Invalid array negative size @virtual_registry_array_create()\n");
        return NULL;
    }
    if (array == NULL && size > 0) {
        DP("ERROR: (parameter) Invalid null VirtualRegistry* array with non-zero size @virtual_registry_array_create()\n");
        return NULL;
    }

    //Aloca a struct na heap, verificando se a operação foi realizada com sucesso
    VirtualRegistryArray *arr = malloc(sizeof(VirtualRegistryArray));
    if (arr == NULL) {
        DP("ERROR: Insuficient memory for VirtualRegistryArray @virtual_registry_array_create()\n");
        return NULL;
    }

    arr->size = size;
    arr->data_arr = array;
    return arr;
}

/*
    Função que desaloca a memória de uma struct de um VirtualRegistryArray.
    OBS: desaloca todos os elementos do vetor contido na struct, bem como o próprio vetor e a struct
    Parâmetros:
        array_ptr -> endereço do array de registros
    Retorno:
        não há retorno
*/
void virtual_registry_array_delete(VirtualRegistryArray **array_ptr) {
    #define array (*array_ptr)

    if (array_ptr == NULL) {
        DP("ERROR: (parameter) invalid null array reference @virtual_registry_array_delete()\n");
        return;
    }

    for (int i = 0; i < array->size; i++)
        virtual_registry_free(&array->data_arr[i]);
    
    free(array->data_arr);
    array->data_arr = NULL;
    array->size = 0;
    free(array);
    array = NULL;

    #undef array
}


/*
    Funcao que verifica se um registro esta contido em um array de registros, dada uma funcao
    de comparacao de registros, permitindo a versatilidade nas comparacoes, de acordo com as necessidades
    do programa
    Parametros:
        search_terms_array -> o array de registros de referencia
        reg_data -> o registro a ser verificado se esta contido no array
        compare_func -> ponteiro da funcao que sera usada para fazer a comparacao de dois registros
    Retorno:
        bool
        true, caso reg_data esteja em search_terms_array
        false, caso contrario
*/
bool virtual_registry_array_contains(VirtualRegistryArray *reg_data_arr, VirtualRegistry *reg_data, bool (*compare_func)(VirtualRegistry* reg_data1, VirtualRegistry* reg_data2)) {
    if (reg_data_arr == NULL || reg_data == NULL || compare_func == NULL)
        return false;
    
    int size = reg_data_arr->size;

    //Dada a função de comparação informada (function pointer), vê se o registro se compara a algum registro do array
    for (int i = 0; i < size; i++) {
        if ((*compare_func)(reg_data, reg_data_arr->data_arr[i]) == true)
            return true;
    }

    return false;
}