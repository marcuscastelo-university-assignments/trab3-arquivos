#ifndef __REGISTRY_MANAGER__H__
#define __REGISTRY_MANAGER__H__

#include <stdio.h>

#include "bool.h"
#include "registry.h"
#include "registry_array.h"
#include "registry_header.h"

/**
 *  Enum que determina os modos de abertura de arquivo e gerenciamento de status
 *  READ -> abre em rb e não mexe em nenhum byte do arquivo
 *  CREATE -> abre em wb+, criando um novo arquivo, definindo os headers com valores padrão e preenchendo seu lixo (só é feito na criação). Também define o status como '0' até que se finalizem as operações
 *  MODIFY -> abre o arquivo em rb+ para que seja possível fazer atualizações, deleções, etc... Abre o arquivo como se fosse READ, mas define o status como '0'
 */
typedef enum {
    READ = 0, CREATE = 1, MODIFY = 2
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


#define INCREASE -1
#define DECREASE -2

#define FRONT 1
#define BACK -1

typedef struct _registry_manager RegistryManager;

RegistryManager *registry_manager_create(void);
OPEN_RESULT registry_manager_open(RegistryManager *manager, char *bin_filename, OPEN_MODE mode);

void registry_manager_free(RegistryManager **manager_ptr);


void registry_manager_write_current_field_by_num(RegistryManager *manager, VirtualRegistry *reg_data, int field_num);

void registry_manager_update_at(RegistryManager *manager, int RRN, VirtualRegistry *reg_data);

void registry_manager_delete_current (RegistryManager *manager);

//TODO: comentar
typedef void (*RMForeachCallback)(RegistryManager *manager, VirtualRegistry *match_registry);


void registry_manager_write_headers_to_disk(RegistryManager *manager);
void registry_manager_read_headers_from_disk(RegistryManager *manager);

void registry_manager_close(RegistryManager *manager);

void registry_manager_delete(RegistryManager **manager_ptr);

bool registry_manager_is_file_consistent(RegistryManager *manager);

void registry_manager_insert_arr_at_end(RegistryManager *manager, VirtualRegistry **reg_data_arr, int arr_size);
int registry_manager_insert_at_end(RegistryManager *manager, VirtualRegistry *reg_data);

int registry_manager_for_each_match(RegistryManager *manager, VirtualRegistryArray *match_conditions, RMForeachCallback callback_func);

VirtualRegistryArray *registry_manager_fetch(RegistryManager *manager, VirtualRegistry *match_terms);
VirtualRegistry *registry_manager_fetch_at(RegistryManager *manager, int RRN);
VirtualRegistryArray *registry_manager_fetch_all(RegistryManager *manager);

void registry_manager_remove_matches(RegistryManager *manager, VirtualRegistryArray *match_terms_arr);
void registry_manager_remove_at(RegistryManager *manager, int RRN);

void registry_manager_update(RegistryManager *manager, VirtualRegistry *match_terms, VirtualRegistry *new_data);
void registry_manager_update_at(RegistryManager *manager, int RRN, VirtualRegistry *new_data);

void registry_manager_for_each(RegistryManager *manager, RMForeachCallback callback_func);
bool registry_manager_is_empty(RegistryManager *manager);

RegistryHeader *registry_manager_get_registry_header (RegistryManager *manager);

#endif  //!__REGISTRY_MANAGER__H__