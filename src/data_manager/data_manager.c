#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "registry_header.h"
#include "data_manager.h"
#include "binary_io.h"
#include "registry_linked_list.h"
#include "registry_manager.h"

struct _data_manager {
    char *bin_file_name;
    FILE *bin_file;
    OPEN_MODE requested_mode;
    RegistryHeader *header;
    RegistryManager *registry_manager;
};

/**
 *  Factory de DataManager, cria uma instância com valores padrão.
 *  Parâmetros:
 *      char *bin_filename -> nome do arquivo binário a ser aberto/criado mais tarde
 *  Retorno: 
 *      DataManager* -> instância criada 
 */
DataManager *data_manager_create(char *file_name) {
    //Validação de parâmetros
    if (file_name == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null filename @data_manager_create()\n");
        return NULL;
    }

    //Tenta alocar espaço para o DataManager, informando em caso de erro.
    DataManager *data_manager = (DataManager*) malloc(sizeof(DataManager));
    if (data_manager == NULL) {
        fprintf(stderr, "ERROR: not enough memory for DataManager @data_mananger_create!\n");
        return NULL;
    }

    //Define os valores da struct como os padrões
    data_manager->bin_file_name = strdup(file_name);
    data_manager->bin_file = NULL;
    data_manager->registry_manager = NULL;
    data_manager->requested_mode = READ;
    data_manager->header = NULL;

    return data_manager;
}

/**
 *  Escreve os headers no arquivo. Essa função deve ser usada o mínimo possível, uma vez
 *  que escrita a disco é custosa.
 *  Parâmetros:
 *      DataManager *manager -> gerenciador que possui os headers e o arquivo binário aberto em modo que permita a escrita
 *  Retorno: void
 */
void data_manager_write_headers(DataManager *manager) {
    //Validação de parâmetros
    if (manager == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null DataManager @data_manager_write_headers()\n");
        return;
    }

    //Valida o modo, impedindo tentativas de escrita no modo somente leitura
    if (manager->requested_mode == READ) {
        fprintf(stderr, "ERROR: trying to write headers on a read-only DataManager @data_manager_write_headers()\n");
        return;
    }

    //Verifica se o manager não está em um estado inválido, isto é, quando o arquivo não está aberto ou os headers não estão definidos
    if (manager->header == NULL || manager->bin_file == NULL) {
        fprintf(stderr, "ERROR: trying to write headers on a invalid DataManager state @data_manager_write_headers()\n");
        return;
    }

    //Escreve os headers no arquivo binário
    reg_header_write_to_bin(manager->header, manager->bin_file);
}

/**
 *  Lê os headers do arquivo. Essa função deve ser usada o mínimo possível, uma vez
 *  que leitura de disco é custosa.
 *  Parâmetros:
 *      DataManager *manager -> gerenciador que possui os headers e o arquivo binário aberto
 *  Retorno: void
 */
void data_manager_read_headers(DataManager *manager) {
    //Validação de parâmetros
    if (manager == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null DataManager @data_manager_read_headers()\n");
        return;
    }

    if (manager->header == NULL || manager->bin_file == NULL) {
        fprintf(stderr, "ERROR: trying to read headers on a invalid DataManager state @data_manager_read_headers()\n");
        return;
    }

    reg_header_read_from_bin(manager->header, manager->bin_file);
}

/**
 *  Abre ou cria um arquivo binário, o qual será gerenciado pelo DataManager.
 *  Parâmetros:
 *      DataManager *manager -> instância do gerenciador
 *      OPEN_MODE -> READ, WRITE ou MODIFY, indicando o modo de abertura do arquivo
 *  Retorno:
 *      OPEN_RESULT -> resultado da abertura (ler a documentação de OPEN_RESULT)
 * 
 */
OPEN_RESULT data_manager_open(DataManager *manager, OPEN_MODE mode) {
    //Vetor de conversão do enum modo para a string que o representa
    static char* mode_to_str[] = {"rb", "wb+", "rb+"};

    //Validação dos parâmetros, informando o código do problema por meio de um enum
    if (manager == NULL) return OPEN_INVALID_ARGUMENT;

    //Abre o arquivo binário no modo selecionado (ler os modos)
    manager->bin_file = fopen(manager->bin_file_name, mode_to_str[mode]);

    //Se houver um erro na abertura do arquivo, retornar o erro por meio de um enum
    if (manager->bin_file == NULL) return OPEN_FAILED; 

    //Inicializa os headers com valores padrão (ou será usado para a escrita de um novo arquivo, ou substituído pelos headers do arquivo existente)
    manager->header = reg_header_create();

    //Tenta criar um RegistryManager
    manager->registry_manager = registry_manager_create(manager->bin_file, manager->header);
    if (manager->registry_manager == NULL) {
        fprintf(stderr, "ERROR: couldn't create RegistryManager @data_manager_open()\n");
        return OPEN_FAILED;
    }
    
    //Se o modo for WRITE, ou seja, criar um novo arquivo, defina os headers com valores iniciais
    if (mode == WRITE) {
        reg_header_write_to_bin(manager->header, manager->bin_file);
    } else { 
        //Se for outro modo, ou seja, o arquivo já existe, atualize o headers e certifique-se de que o arquivo está consistente e não vazio
        reg_header_read_from_bin(manager->header, manager->bin_file);
        if (reg_header_get_status(manager->header) != '1') return OPEN_INCONSISTENT;
        if (reg_header_get_registries_count(manager->header) == 0) return OPEN_EMPTY;

        if (mode == MODIFY) { //Se houver intenção de modificar o arquivo, defina o status como inconsistente
            reg_header_set_status(manager->header, '0');
            reg_header_write_to_bin(manager->header, manager->bin_file);
        }
    }

    //Guarda o modo de abertura para uso em outras funções
    manager->requested_mode = mode;
    
    return OPEN_OK;
}

/**
 *  Fecha o arquivo binário, limpando a memória de quaisquer estruturas auxiliares utilizadas
 *  Parâmetros:
 *      DataManager *manager -> gerenciador que possui o arquivo aberto
 *  Retorno: void
 */
void data_manager_close(DataManager *manager) {
    //Verifica se o manager já foi deletado ou se o arquivo já foi fechado
    if (manager == NULL || manager->bin_file == NULL) return;
    
    //Marca o arquivo como consistente. (OBS: não é necessário no caso da leitura, pois nenhuma modificação foi feita)
    if (manager->requested_mode != READ) {
        reg_header_set_status(manager->header, '1');
        data_manager_write_headers(manager);
    }
    reg_header_delete(&manager->header);

    //Libera a memória utilizada pelo RegistryManager e fecha o arquivo
    registry_manager_delete(&manager->registry_manager);
    fclose(manager->bin_file);
    manager->bin_file = NULL;
    manager->registry_manager = NULL;
}


/**
 *  Deleta o DataManager, ou seja, libera toda a memória por ele utilizada.
 *  OBS: se o arquivo não tiver sido fechado anteriormente, ele é fechado nessa função
 *  Parâmetros:
 *      DataManager **manager_ptr -> referência do pointer usado pelo programador.
 *  Retorno: void
 */
void data_manager_delete(DataManager **manager_ptr) {
    #define manager (*manager_ptr)

    //Validação do parâmetro. Deve ser referência ao pointer que aponta para o DataManager
    if (manager_ptr == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null pointer @data_manager_delete()\n");
        return;
    }

    //Fecha o DataManager se já não tiver sido fechado. Libera a memória usada pelo DataManager
    data_manager_close(manager);
    registry_manager_delete(&manager->registry_manager);
    free(manager->bin_file_name);
    free(manager);
    manager = NULL;

    #undef manager
}

/**
 *  Adiciona um vetor de VirtualRegistries ao fim do arquivo binário
 *  Dessa forma, menos atualizações são feitas, pois o programa já sabe que
 *  serão inseridos diversos registros.
 *  Parâmetros:
 *      DataManager *manager -> gerenciador que possui o arquivo referido aberto
 *      VirtualRegistry *reg_arr -> vetor de registros a serem inseridos
 *      int arr_size -> tamanho do vetor
 *  Retorno: void
 */
void data_manager_insert_arr_at_end(DataManager *manager, VirtualRegistry **reg_arr, int arr_size) {
    //Valida o estado atual com um manager instanciado e o arquivo aberto
    if (manager == NULL || manager->bin_file == NULL) {
        fprintf(stderr, "ERROR: invalid DataManager state! @data_manager_insert_at_end\n");
        return;
    }

    //O arquivo deve ter sido aberto em um modo que permita a escrita
    if (manager->requested_mode == READ) {
        fprintf(stderr, "ERROR: DataManager is in read-only mode @data_manager_insert_at_end\n");
        return;
    }

    //Posiciona o cursor do arquivo ao fim do arquivo
    registry_manager_seek_end(manager->registry_manager);

    //Escreve diversos registros
    for (int i = 0; i < arr_size; i++) {
        VirtualRegistry *curr_reg_data = reg_arr[i];
        registry_manager_write_current(manager->registry_manager, curr_reg_data);
    }

    //Atualiza apenas ao fim de toda a operação o próximo RRN
    reg_header_set_next_RRN(manager->header, reg_header_get_next_RRN(manager->header) + arr_size);
    reg_header_set_registries_count(manager->header, reg_header_get_registries_count(manager->header) + arr_size);
}


/**
 *  Adiciona um registro ao fim do arquivo.
 *  Parâmetros:
 *      DataManager *manager -> gerenciador que possui o arquivo referido aberto
 *  Retorno: int - RRN no qual o registro foi inserido
 */
int data_manager_insert_at_end(DataManager *manager, VirtualRegistry *reg_data) {
    //Inserir um registro é apenas um caso especial de inserir um vetor de tamanho 1
    data_manager_insert_arr_at_end(manager, &reg_data, 1);
    return reg_header_get_next_RRN(manager->header) - 1;
}



/**
 *  Busca todos os registros no arquivo que condigam com os termos de busca.
 *  OBS: os termos de busca são especificados com uma máscara, indicando quais campos
 *  devem ser comparados e quais são irrelevantes, de acordo com a entrada do usuário.
 *  Parâmetros:
 *      DataManager *manager -> gerenciador que tem o arquivo aberto (pode ser modo leitura também)
 *  Retorno:
 *      VirtualRegistryArray* -> vetor com todos os registros encontrados de acordo com os termos de busca
 */
VirtualRegistryArray *data_manager_fetch(DataManager *manager, VirtualRegistry *search_terms) {
    //Validação de parâmetros
    if (manager == NULL || search_terms == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid parameters @data_manager_fetch()\n");
        return NULL;
    }

    //Validação de estado do arquivo binário
    if (manager->bin_file == NULL) {
        fprintf(stderr, "ERROR: DataManager haven't opened the binary file @data_manager_fetch()\n");
        return NULL;
    }

    int registriesCounter = reg_header_get_registries_count(manager->header);

    //Tenta criar uma lista ligada na qual serão inseridos os registros que condizerem com os termos de busca
    RegistryLinkedList *list = registry_linked_list_create();
    if (list == NULL) {
        fprintf(stderr, "ERROR: couldn't create RegistryLinkedList @data_manager_fetch()\n");
        return NULL;
    }

    //Se não houverem registros, não há porque der fseek
    if (registriesCounter > 0)
        registry_manager_seek_first(manager->registry_manager);

    //Para todos os registros, adicione-os a lista caso se encaixem aos termos de busca
    for (int i = 0; i < registriesCounter; ) {
        //Obtém o registro na posição do cursor e avança para o próximo
        VirtualRegistry *reg_data = registry_manager_read_current(manager->registry_manager);
        
        if (reg_data == NULL) continue;

        //Verifica se, dentro da máscara informada, o registro atual atende aos termos de busca e o adiciona na lista em caso positivo
        if (virtual_registry_compare(reg_data, search_terms) == true)
            registry_linked_list_insert(list, reg_data);
        else //Se não, libera a memória neste momento, já que não haverá mais referências ao registro 
            virtual_registry_delete(&reg_data);

        //i++ para registros não deletados
        i++;
    }
    
    //Converte a lista ligada para uma struct que guarda um vetor e seu tamanho
    VirtualRegistryArray *reg_data_array = registry_linked_list_to_array(list);

    //Libera a memória da lista ligada, sem apagar os registros, já que estes serão usados no vetor acima criado
    registry_linked_list_delete(&list, false);

    return reg_data_array;
}


/**
 *  Obtém um registro dado um RRN.
 *  Parâmetros:
 *      DataManager *manager -> gerenciador que possui o arquivo aberto
 *  Retorno:
 *      VirtualRegistry* -> Registro encontrado no RRN especificado ou NULL se não encontrado
 * 
 */
VirtualRegistry *data_manager_fetch_at(DataManager *manager, int RRN) {
    //Validação de parâmetros
    if (manager == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null DataManager @data_manager_fetch_all()\n");
        return NULL;
    }

    //Validação do estado do arquivo binário
    if (manager->bin_file == NULL) {
        fprintf(stderr, "ERROR: DataManager haven't opened the binary file @data_manager_fetch_all()\n");
        return NULL;
    } 

    //Indica que o registro é inexistente se o RRN for inexistente
    if (reg_header_get_next_RRN(manager->header) <= RRN) return NULL;

    return registry_manager_read_at(manager->registry_manager, RRN);
}


/**
 *  Obtém todos os registros do arquivo em forma de vetor.
 *  Parâmetros:
 *      DataManager *manager -> gerenciador com o arquivo aberto.
 *  Retorno: 
 *      VirtualRegistryArray* -> vetor com os registros
 */
VirtualRegistryArray *data_manager_fetch_all(DataManager *manager) {
    //Validação de parâmetros
    if (manager == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null DataManager @data_manager_fetch_at()\n");
        return NULL;
    }

    //Validação do estado do arquivo binário
    if (manager->bin_file == NULL) {
        fprintf(stderr, "ERROR: DataManager haven't opened the binary file @data_manager_fetch_at()\n");
        return NULL;
    } 

    #define reg_man manager->registry_manager

    int arr_size = reg_header_get_registries_count(manager->header);

    //Se não houver registros, retorna um vetor vazio
    if (arr_size == 0) return virtual_registry_array_create(NULL, 0);

    VirtualRegistry **reg_arr = (VirtualRegistry**) malloc(sizeof(VirtualRegistry*) * arr_size);
    VirtualRegistry *reg_data;

    //Posiciona o cursor no primeiro registro
    registry_manager_seek_first(reg_man);

    //Alimenta o vetor com os registros lidos do arquivo
    for (int i = 0; i < arr_size; i++) {
        reg_data = registry_manager_read_current(reg_man);

        //Ignora registros deletados
        if (reg_data != NULL) reg_arr[i] = reg_data;   
    }

    return virtual_registry_array_create(reg_arr, arr_size);;

    #undef reg_man
}

/**
 *  Remove registros que se encaixem em um dos termos especificados.
 *  Parâmetros:
 *      DataManager *manager -> gerenciador que  possui o arquivo aberto
 *      VirtualRegistryArray *search_terms_array -> vetor de termos de busca
 *  Retorno: void
 */
void data_manager_remove_matches (DataManager *manager, VirtualRegistryArray *search_terms_arr) {
    #define reg_man manager->registry_manager

    //Validação de parâmetros
    if (manager == NULL || search_terms_arr == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid parameter @data_manager_remove_matches()\n");
        return;
    }

    int cur_reg_count = reg_header_get_registries_count(manager->header);
    int cur_del_count = reg_header_get_removed_count(manager->header);
    int new_del_count = 0;

    //Garante que existem registros para sererm removidos
    if (cur_reg_count <= 0) return;

    //Move o cursor para o primeiro registro
    registry_manager_seek_first(reg_man);

    for (int i = 0; i < cur_reg_count;) {
        VirtualRegistry *reg_data = registry_manager_read_current(reg_man);

        //Se o registro era um deletado, não conte i++
        if (reg_data == NULL) continue;

        //Verifica se o registro atual se encaixa em um dos termos de busca. Se sim, apague-o
        if (virtual_registry_array_contains(search_terms_arr, reg_data, virtual_registry_compare) == true) {
            //Volta o cursor para o registro, já que ele foi deslocado devido à leitura
            registry_manager_jump_registry(reg_man, BACK);

            //Apaga do disco
            registry_manager_delete_current(reg_man);
            new_del_count++;
        }

        //Libera a memória do registro na RAM
        virtual_registry_delete(&reg_data);

        //i++ somente se o registro não era deletado
        i++;
    }

    //Atualiza os headers (apenas na RAM, por enquanto)
    reg_header_set_removed_count(manager->header, cur_del_count + new_del_count);
    reg_header_set_registries_count(manager->header, cur_reg_count - new_del_count);

    #undef reg_man
} 

/**
 *  Atualiza no disco um registro dado um RRN
 *  Parâmetros:
 *      DataManager *manager -> gerenciador com o arquivo aberto em modo que permita escrita
 *      int RRN -> RRN no qual o registro se encontra
 *      VirtualRegistryUpdater *new_data -> registro com máscara de bits indicando quais campos devem ser atualizados
 *  Retorno: void 
 */
void data_manager_update_at(DataManager *manager, int RRN, VirtualRegistryUpdater *new_data) {
    //Validação de parâmetros
    if (manager == NULL || new_data == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null parameters @data_manager_update_at()\n");
        return;
    }

    //Validação do estado do arquivo
    if (manager->bin_file == NULL || manager->registry_manager == NULL) {
        fprintf(stderr, "ERROR: DataManager is in an invalid state @data_manager_update_at()\n");
        return;
    }

    if (manager->requested_mode == READ) {
        fprintf(stderr, "ERROR: DataManager is in read-only mode @data_manager_update_at()\n");
        return;
    }


    if (reg_header_get_next_RRN(manager->header) <= RRN) return;

    registry_manager_seek(manager->registry_manager, RRN);

    if (registry_manager_is_current_deleted(manager->registry_manager) == false) {
        registry_manager_update_current(manager->registry_manager, new_data);
        reg_header_set_updated_count(manager->header, H_INCREASE);
    }
}   