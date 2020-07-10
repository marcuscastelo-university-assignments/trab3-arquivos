#ifndef __DATA_MANAGER__H__
#define __DATA_MANAGER__H__

#include <stdio.h>

#include "bool.h"
#include "registry.h"
#include "registry_array.h"
#include "registry_header.h"




/**
 *  Enum que determina os modos de abertura de arquivo e gerenciamento de status
 *  READ -> abre em rb e não mexe em nenhum byte do arquivo
 *  WRITE -> abre em wb+, criando um novo arquivo, definindo os headers com valores padrão e preenchendo seu lixo (só é feito na criação). Também define o status como '0' até que se finalizem as operações
 *  MODIFY -> abre o arquivo em rb+ para que seja possível fazer atualizações, deleções, etc... Abre o arquivo como se fosse READ, mas define o status como '0'
 */
typedef enum {
    READ = 0, WRITE = 1, MODIFY = 2
} OPEN_MODE;

/**
 *  Enum que indica o resultado da abertura do arquivo
 *  OPEN_OK -> indica que não houve problema na abertura do arquivo
 *  OPEN_FAILED -> Indica a necessidade de exibir a mensagem de erro "Falha no processamento do arquivo.", de acordo com as especificações do trabalho
 *  OPEN_INCONSISTENT ->  Indica que o arquivo está inconsistente, exibe "Falha no processamento do arquivo.", de acordo com as especificações do trabalho
 *  OPEN_EMPTY -> Indica que não há registros no arquivo aberto, com a mensagem "Registro Inexistente.", de acordo com as especificações do trabalho
 *  OPEN_INVALID_ARGUMENT
 */
typedef enum {
    OPEN_OK = 0, OPEN_FAILED = 1, OPEN_INCONSISTENT = 2, OPEN_EMPTY = 4, OPEN_INVALID_ARGUMENT = 8
} OPEN_RESULT;

typedef struct _data_manager DataManager;

//TODO: comentar
typedef void (*DMForeachCallback)(DataManager *manager, VirtualRegistry *match_registry);

DataManager *data_manager_create(char *file_name);

OPEN_RESULT data_manager_open(DataManager *manager, OPEN_MODE mode);

void data_manager_write_headers_to_disk(DataManager *manager);
void data_manager_read_headers_from_disk(DataManager *manager);

void data_manager_close(DataManager *manager);

void data_manager_delete(DataManager **manager_ptr);

bool data_manager_is_file_consistent(DataManager *manager);

void data_manager_insert_arr_at_end(DataManager *manager, VirtualRegistry **reg_data_arr, int arr_size);
int data_manager_insert_at_end(DataManager *manager, VirtualRegistry *reg_data);

void data_manager_for_each_match(DataManager *manager, VirtualRegistryArray *match_conditions, DMForeachCallback callback_func);

VirtualRegistryArray *data_manager_fetch(DataManager *manager, VirtualRegistry *match_terms);
VirtualRegistry *data_manager_fetch_at(DataManager *manager, int RRN);
VirtualRegistryArray *data_manager_fetch_all(DataManager *manager);

void data_manager_remove_matches(DataManager *manager, VirtualRegistryArray *match_terms_arr);
void data_manager_remove_at(DataManager *manager, int RRN);

void data_manager_update(DataManager *manager, VirtualRegistry *match_terms, VirtualRegistry *new_data);
void data_manager_update_at(DataManager *manager, int RRN, VirtualRegistry *new_data);

void data_manager_for_each(DataManager *manager, DMForeachCallback callback_func);
bool data_manager_is_empty(DataManager *manager);

RegistryHeader *data_manager_get_registry_header (DataManager *manager);

#endif  //!__DATA_MANAGER__H__