#include "registry_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "binary_io.h"
#include "binary_registry.h"
#include "registry_header.h"
#include "registry_utils.h"
#include "registry.h"
#include "string_utils.h"
#include "debug.h"
#include "registry_linked_list.h"

#define REG_SIZE 128

/*
	Struct que representa o gerenciador do arquivo de registros, usada para
	armazenar certas informações relacionadas ao arquivo, como os headers, 
	modo de abertura e rrn atual.
*/
struct _registry_manager {
    FILE *bin_file;
    OPEN_MODE requested_mode;
    RegistryHeader *header;
	int currRRN;			//RRN atual do ponteiro
};


/**
 *  Factory de RegistryManager, cria uma instância com valores padrão.
 *  Parâmetros: nenhum
 *  Retorno: 
 *      RegistryManager* -> instância criada 
 */
RegistryManager *registry_manager_create(void) {
    //Tenta alocar espaço para o RegistryManager, informando em caso de erro.
    RegistryManager *registry_manager = (RegistryManager*) malloc(sizeof(RegistryManager));
    if (registry_manager == NULL) {
        DP("ERROR: not enough memory for RegistryManager @data_mananger_create!\n");
        return NULL;
    }

    //Define os valores da struct como os padrões
    registry_manager->bin_file = NULL;
    registry_manager->requested_mode = READ;
    registry_manager->header = NULL;
	registry_manager->currRRN = -1;

    return registry_manager;
}

/**
 *  Abre ou cria um arquivo binário, o qual será gerenciado pelo RegistryManager.
 *  Parâmetros:
 *      RegistryManager *manager -> instância do gerenciador
 *		char* bin_filename -> nome do arquivo (caminho completo)
 *      OPEN_MODE -> READ, CREATE ou MODIFY, indicando o modo de abertura do arquivo
 *  Retorno:
 *      OPEN_RESULT -> resultado da abertura (ler a documentação de OPEN_RESULT em open_mode.h)
 * 
 */
OPEN_RESULT registry_manager_open(RegistryManager *manager, char* bin_filename, OPEN_MODE mode) {
	//Validação dos parâmetros, informando o código do problema por meio de um enum
    if (manager == NULL) return OPEN_INVALID_ARGUMENT;

	//Validação de parâmetros
    if (bin_filename == NULL) {
        DP("ERROR: (parameter) invalid null filename @registry_manager_open()\n");
        return OPEN_INVALID_ARGUMENT;
    }

	if (DEBUG && manager->bin_file != NULL) {
		DP("WARNING: opening new file before closing file already opened! @registry_manager_open()\n");
		return OPEN_INVALID_ARGUMENT;
	}

    //Vetor de conversão do enum modo para a string que o representa (READ = rb, CREATE = )
    static char* mode_to_str[] = {"rb", "wb+", "rb+"};

    //Guarda o modo de abertura para uso em outras funções
    manager->requested_mode = mode;

    //Abre o arquivo binário no modo selecionado (ler os modos)
    manager->bin_file = fopen(bin_filename, mode_to_str[mode]);

    //Se houver um erro na abertura do arquivo, retornar o erro por meio de um enum
    if (manager->bin_file == NULL) return OPEN_FAILED; 

    //Inicializa os headers com valores padrão (ou será usado para a escrita de um novo arquivo, ou substituído pelos headers do arquivo existente)
    manager->header = reg_header_create();
    
    //Se o modo for CREATE, ou seja, criar um novo arquivo, defina os headers com valores iniciais (RAM -> disco)
    if (mode == CREATE) {
        reg_header_write_to_bin(manager->header, manager->bin_file);
    } else { 
        //Se for outro modo, ou seja, o arquivo já existe, atualize o headers (disco -> RAM) e certifique-se de que o arquivo está consistente e não vazio
        reg_header_read_from_bin(manager->header, manager->bin_file);
        if (reg_header_get_status(manager->header) != '1') return OPEN_INCONSISTENT;

        if (mode == MODIFY) { 
			//Se houver intenção de modificar o arquivo, defina o status como inconsistente
            reg_header_set_status(manager->header, '0');
            reg_header_write_to_bin(manager->header, manager->bin_file);
        }
    }
    
    return OPEN_OK;
}

/**
 *  Fecha o arquivo binário, limpando a memória de quaisquer estruturas auxiliares utilizadas
 *  Parâmetros:
 *      RegistryManager *manager -> gerenciador que possui o arquivo aberto
 *  Retorno: void
 */
void registry_manager_close(RegistryManager *manager) {
    //Verifica se o manager já foi deletado ou se o arquivo já foi fechado
    if (manager == NULL || manager->bin_file == NULL) return;
    
    if (manager->requested_mode != READ) {
		//Marca o arquivo como consistente. (OBS: não é necessário no caso da leitura, pois nenhuma modificação foi feita)
        reg_header_set_status(manager->header, '1');
		//Salva os headers no disco
        registry_manager_write_headers_to_disk(manager);
    }

	//Limpa a memória dos headers na RAM
    reg_header_delete(&manager->header);

    //fecha o arquivo
    fclose(manager->bin_file);
	//Marca qua não existe arquivo aberto
    manager->bin_file = NULL;

	manager->currRRN = -1;
}


/**
 *  Destroi o RegistryManager, ou seja, libera toda a memória por ele utilizada.
 *  OBS: se o arquivo não tiver sido fechado anteriormente, ele é fechado nessa função
 *  Parâmetros:
 *      RegistryManager **manager_ptr -> referência do pointer usado pelo programador.
 *  Retorno: void
 */
void registry_manager_free(RegistryManager **manager_ptr) {
    #define manager (*manager_ptr)

    //Validação do parâmetro. Deve ser referência ao pointer que aponta para o RegistryManager
    if (manager_ptr == NULL) {
        DP("ERROR: (parameter) invalid null pointer @registry_manager_free()\n");
        return;
    }

    //Fecha o arquvo aberto pelo RegistryManager se já não tiver sido fechado.
    registry_manager_close(manager);

    free(manager);
    manager = NULL;

    #undef manager
}


/*
	Funcao (privada) para fazer seek no arquivo sendo gerenciado
	Parametros:
		manager -> o gerenciador que contem o arquivo onde sera feito o seek
		RRN -> RRN para onde o ponteiro sera levado
	Retorno: void
*/
static void _seek_registry(RegistryManager *manager, int RRN) {
	//Validação de parâmetros
	if (manager == NULL) {
		DP("ERROR: invalid parameter @_seek_registry()\n");
		return;
	}
	
	if (RRN < 0) {
		DP("ERROR: invalid given RRN @_seek_registry()\n");
		return;
	}

	fseek(manager->bin_file, (RRN+1) * REG_SIZE, SEEK_SET);
	manager->currRRN = RRN;
}

/*
	Funcao que faz fseek para o primeiro registro (RRN = 0)
	Parametros:
		manager -> o gerenciador que contem o arquivo onde sera feito o seek
	Retorno: void
*/
static void _seek_first_registry(RegistryManager *manager) { _seek_registry(manager, 0); }


/*
	Funcao que faz fseek para a posição após o ultimo registro (para, por exemplo, a inserção de um novo registro)
	Parametros:
		manager -> o gerenciador que contem o arquivo onde sera feito o seek
	Retorno: void
*/
static void _seek_new_registry(RegistryManager *manager) { _seek_registry(manager, reg_header_get_next_RRN(manager->header)); }

/*
	Funcao que le um registro, precisa estar exatamente no comeco do registro para funcionar
	Parametros:
		manager -> o gerenciador de registro que tera' um registro lido em seu binario
	Retorno:
		VirtualRegistry* -> O registro lido. NULL caso o registro tenha sido removido
*/
static VirtualRegistry *_read_current_registry(RegistryManager *manager) {
	if (manager == NULL) {
		DP("ERROR: invalid parameter @_read_current_registry()\n");
		return NULL;
	}

	manager->currRRN++;
	return binary_read_registry(manager->bin_file);
}


/*
	Funcao que le um registro em um RRN dado
	Parametros:
		manager -> o gerenciador de registro que tera' um registro lido em seu binario
		RRN -> o RRN do registro a ser lido
	Retorno:
		VirtualRegistry* -> O registro lido. NULL caso o registro tenha sido removido
*/
static VirtualRegistry *_read_registry_at(RegistryManager *manager, int RRN) {
	if (manager == NULL) {
		DP("ERROR: invalid parameter @_read_registry_at()\n");
		return NULL;
	}
	
	_seek_registry(manager, RRN);			//faz o seek do RRN
	return _read_current_registry(manager);	//le o registro e retorna
}


/*
	Funcao que escreve um registro onde o ponteiro esta. Precisa estar exatamente no 
	comeco de um RRN para ser eficiente
	Paramentros:
		manager -> o gerenciador de registro que tera' um registro escrito em seu binario
		reg_data -> o registro que sera escrito
	Retorno: void
*/
static void _write_current_registry(RegistryManager *manager, VirtualRegistry *reg_data) {
	if (manager == NULL || reg_data == NULL) {
		DP("ERROR: invalid parameter @_write_current_registry()\n");
		return;
	}

	registry_prepare_for_write(reg_data);					//faz o tratamento de campos invalidos
	binary_write_registry(manager->bin_file, reg_data);		//escreve no binario
	manager->currRRN++;
}

/*
	Funcao que deleta o registro onde o ponteiro esta'
	Precisa estar exatamente no comeco do registro para ser eficiente
	Parametros:
		manager -> o gerenciador de registro que tera' o registro deletado em seu binario
	Retorno: void
*/
static void _delete_current_registry (RegistryManager *manager) {
	if (manager == NULL) {
		DP("ERROR: invalid parameter @_delete_current_registry()\n");
		return;
	}
	binary_write_int(manager->bin_file, -1);	//escreve o indicador de registro deletado: -1
	fseek(manager->bin_file, REG_SIZE-sizeof(int), SEEK_CUR); //faz o seek para ir para o final do registro
	manager->currRRN++;
}

/*
	Funcao que pula os bytes de um registro (128 bytes) em uma determinada direcao
	Parametros:
		manager -> o gerenciador de registros que tera seu ponteiro alterado
		direction: 
				  -> FRONT : pula +128 bytes (expands to +1)
				  -> BACK  : pula -128 bytes (expands to -1)
	Retorno: void
*/
static void _jump_registry (RegistryManager *manager, int direction) {
	if (manager == NULL) {
		DP("ERROR: invalid parameter @_jump_registry()\n");
		return;
	}

	fseek(manager->bin_file, REG_SIZE*direction, SEEK_CUR); //faz o seek para de +-128 bytes, pulando um registro pra frente ou pra tras 
	manager->currRRN += direction;
}





/*
	Faz o update de um registro onde o ponteiro esta'
	Precisa estar no comeco de um registro para ser eficiente
	Parametros:
		manager -> o gerenciador de registros que tera' um registro de seu binario atualizado
		reg_data -> registro que contem os valores que serao alterados
	Retorno:
		nao ha retorno
*/
static bool _update_current_registry(RegistryManager *manager, VirtualRegistry *new_data) {
	if (manager == NULL || new_data == NULL) {
		DP("ERROR: invalid parameter @_update_current_registry()\n");
		return false;
	}

	registry_prepare_for_write(new_data);
	return binary_update_registry(manager->bin_file, new_data) == true;
}























/**
 *  Escreve os headers no arquivo. Essa função deve ser usada o mínimo possível, uma vez
 *  que escrita a disco é custosa.
 *  Parâmetros:
 *      RegistryManager *manager -> gerenciador que possui os headers e o arquivo binário aberto em modo que permita a escrita
 *  Retorno: void
 */
void registry_manager_write_headers_to_disk(RegistryManager *manager) {
    //Validação de parâmetros
    if (manager == NULL) {
        DP("ERROR: (parameter) invalid null RegistryManager @registry_manager_write_headers_to_disk()\n");
        return;
    }

    //Valida o modo, impedindo tentativas de escrita no modo somente leitura
    if (manager->requested_mode == READ) {
        DP("ERROR: trying to write headers on a read-only RegistryManager @registry_manager_write_headers_to_disk()\n");
        return;
    }

    //Verifica se o manager não está em um estado inválido, isto é, quando o arquivo não está aberto ou os headers não estão definidos
    if (manager->header == NULL || manager->bin_file == NULL) {
        DP("ERROR: trying to write headers on a invalid RegistryManager state @registry_manager_write_headers_to_disk()\n");
        return;
    }

    //Escreve os headers no arquivo binário
    reg_header_write_to_bin(manager->header, manager->bin_file);
}

/**
 *  Lê os headers do arquivo. Essa função deve ser usada o mínimo possível, uma vez
 *  que leitura de disco é custosa.
 *  Parâmetros:
 *      RegistryManager *manager -> gerenciador que possui os headers e o arquivo binário aberto
 *  Retorno: void
 */
void registry_manager_read_headers_from_disk(RegistryManager *manager) {
    //Validação de parâmetros
    if (manager == NULL) {
        DP("ERROR: (parameter) invalid null RegistryManager @registry_manager_read_headers_from_disk()\n");
        return;
    }

    if (manager->header == NULL || manager->bin_file == NULL) {
        DP("ERROR: trying to read headers on a invalid RegistryManager state @registry_manager_read_headers_from_disk()\n");
        return;
    }

    reg_header_read_from_bin(manager->header, manager->bin_file);
}





/**
 *  Adiciona um vetor de VirtualRegistries ao fim do arquivo binário
 *  Dessa forma, menos atualizações são feitas, pois o programa já sabe que
 *  serão inseridos diversos registros.
 *  Parâmetros:
 *      RegistryManager *manager -> gerenciador que possui o arquivo referido aberto
 *      VirtualRegistry *reg_arr -> vetor de registros a serem inseridos
 *      int arr_size -> tamanho do vetor
 *  Retorno: void
 */
void registry_manager_insert_arr_at_end(RegistryManager *manager, VirtualRegistry **reg_arr, int arr_size) {
    //Valida o estado atual com um manager instanciado e o arquivo aberto
    if (manager == NULL || manager->bin_file == NULL) {
        DP("ERROR: invalid RegistryManager state! @registry_manager_insert_at_end\n");
        return;
    }

    //O arquivo deve ter sido aberto em um modo que permita a escrita
    if (manager->requested_mode == READ) {
        DP("ERROR: RegistryManager is in read-only mode @registry_manager_insert_at_end\n");
        return;
    }

    //Posiciona o cursor do arquivo ao fim do arquivo
    _seek_new_registry(manager);

    //Escreve diversos registros
    for (int i = 0; i < arr_size; i++) {
        VirtualRegistry *curr_reg_data = reg_arr[i];
        _write_current_registry(manager, curr_reg_data);
    }

    //Atualiza apenas ao fim de toda a operação o próximo RRN
    reg_header_set_next_RRN(manager->header, reg_header_get_next_RRN(manager->header) + arr_size);
    reg_header_set_registries_count(manager->header, reg_header_get_registries_count(manager->header) + arr_size);
}


/**
 *  Adiciona um registro ao fim do arquivo.
 *  Parâmetros:
 *      RegistryManager *manager -> gerenciador que possui o arquivo referido aberto
 *  Retorno: int - RRN no qual o registro foi inserido
 */
int registry_manager_insert_at_end(RegistryManager *manager, VirtualRegistry *reg_data) {
    //Inserir um registro é apenas um caso especial de inserir um vetor de tamanho 1
    registry_manager_insert_arr_at_end(manager, &reg_data, 1);
    return reg_header_get_next_RRN(manager->header) - 1;
}



/**
 *  Busca todos os registros no arquivo que condigam com os termos de busca.
 *  OBS: os termos de busca são especificados com uma máscara, indicando quais campos
 *  devem ser comparados e quais são irrelevantes, de acordo com a entrada do usuário.
 *  Parâmetros:
 *      RegistryManager *manager -> gerenciador que tem o arquivo aberto (pode ser modo leitura também)
 *  Retorno:
 *      VirtualRegistryArray* -> vetor com todos os registros encontrados de acordo com os termos de busca
 */
VirtualRegistryArray *registry_manager_fetch(RegistryManager *manager, VirtualRegistry *search_terms) {
    //Tenta criar uma lista ligada na qual serão inseridos os registros que condizerem com os termos de busca
    RegistryLinkedList *list = registry_linked_list_create();
    if (list == NULL) {
        DP("ERROR: couldn't create RegistryLinkedList @registry_manager_fetch()\n");
        return NULL;
    }


    RMForeachCallback innerCallback = ({
        void _callback(RegistryManager *manager, VirtualRegistry *registry) {
            //Verifica se, dentro da máscara informada, o registro atual atende aos termos de busca e o adiciona na lista em caso positivo
            if (virtual_registry_compare(registry, search_terms) == true)
                registry_linked_list_insert(list, virtual_registry_create_copy(registry));
        } _callback;
    });

    registry_manager_for_each(manager, innerCallback);        

    //Converte a lista ligada para uma struct que guarda um vetor e seu tamanho
    VirtualRegistryArray *reg_data_array = registry_linked_list_to_array(list);

    //Libera a memória da lista ligada, sem apagar os registros, já que estes serão usados no vetor acima criado
    registry_linked_list_delete(&list, false);

    return reg_data_array;
}


/**
 *  Obtém um registro dado um RRN.
 *  Parâmetros:
 *      RegistryManager *manager -> gerenciador que possui o arquivo aberto
 *  Retorno:
 *      VirtualRegistry* -> Registro encontrado no RRN especificado ou NULL se não encontrado (deve ser liberado manualmente)
 * 
 */
VirtualRegistry *registry_manager_fetch_at(RegistryManager *manager, int RRN) {
    //Validação de parâmetros
    if (manager == NULL) {
        DP("ERROR: (parameter) invalid null RegistryManager @registry_manager_fetch_at()\n");
        return NULL;
    }

    //Validação do estado do arquivo binário
    if (manager->bin_file == NULL) {
        DP("ERROR: RegistryManager haven't opened the binary file @registry_manager_fetch_at()\n");
        return NULL;
    } 

    //Indica que o registro é inexistente se o RRN for inexistente
    if (reg_header_get_next_RRN(manager->header) <= RRN || RRN < 0) return NULL;

    return _read_registry_at(manager, RRN);
}


/**
 *  Busca todos os registros no arquivo que condigam com os termos de busca.
 *  OBS: os termos de busca são especificados com uma máscara, indicando quais campos
 *  devem ser comparados e quais são irrelevantes, de acordo com a entrada do usuário.
 *  Parâmetros:
 *      RegistryManager *manager -> gerenciador que tem o arquivo aberto (pode ser modo leitura também)
 *  Retorno:
 *      int -> número de registros encontrados
 */
int registry_manager_for_each_match(RegistryManager *manager, VirtualRegistryArray *match_conditions, RMForeachCallback callback_func) {
    int foundRegistries = 0;

    if (callback_func == NULL) {
        DP("ERROR: calling registry_manager_for_each_match without callback function\n");
        return -1;
    }

    //Validação de parâmetros
    if (manager == NULL) {
        DP("ERROR: (parameter) invalid parameter @registry_manager_remove_matches()\n");
        return -1;
    }

    //Garante que existem registros para sererm removidos
    if (registry_manager_is_empty(manager)) return 0;

    //Move o cursor para o primeiro registro
    _seek_first_registry(manager);

    int reg_count = reg_header_get_registries_count(manager->header) + reg_header_get_removed_count(manager->header);
    for (int i = 0; i < reg_count; i++) {
        VirtualRegistry *reg_data = _read_current_registry(manager);

        if (reg_data == NULL) continue;

        //Verifica se o registro atual se encaixa em um dos termos de busca. Se sim, chame o callback
        if (match_conditions == NULL || virtual_registry_array_contains(match_conditions, reg_data, virtual_registry_compare) == true) {
            callback_func(manager, reg_data);
			foundRegistries++;
        }

        //Libera a memória do registro na RAM
        virtual_registry_free(&reg_data);
    }

	return foundRegistries;
}

void _DMForeachCallback_remove(RegistryManager *manager, VirtualRegistry *reg) {
    //Supõe-se que o registro recebido já foi lido e por isso o cursor se encontra um registro além
    _jump_registry(manager, BACK);
    _delete_current_registry(manager);
    reg_header_set_removed_count(manager->header, H_INCREASE);
    reg_header_set_registries_count(manager->header, H_DECREASE);
}

/**
 *  Remove registros que se encaixem em um dos termos especificados.
 *  Parâmetros:
 *      RegistryManager *manager -> gerenciador que  possui o arquivo aberto
 *      VirtualRegistryArray *search_terms_array -> vetor de termos de busca
 *  Retorno: void
 */
void registry_manager_remove_matches (RegistryManager *manager, VirtualRegistryArray *match_terms_arr) {
    registry_manager_for_each_match(manager, match_terms_arr, _DMForeachCallback_remove);
} 

/**
 *  Atualiza no disco um registro dado um RRN
 *  Parâmetros:
 *      RegistryManager *manager -> gerenciador com o arquivo aberto em modo que permita escrita
 *      int RRN -> RRN no qual o registro se encontra
 *      VirtualRegistryUpdater *new_data -> registro com máscara de bits indicando quais campos devem ser atualizados
 *  Retorno: void 
 */
void registry_manager_update_at(RegistryManager *manager, int RRN, VirtualRegistryUpdater *new_data) {
    //Validação de parâmetros
    if (manager == NULL || new_data == NULL) {
        DP("ERROR: (parameter) invalid null parameters @registry_manager_update_at()\n");
        return;
    }

    //Validação do estado do arquivo
    if (manager->bin_file == NULL || manager == NULL) {
        DP("ERROR: RegistryManager is in an invalid state @registry_manager_update_at()\n");
        return;
    }

    if (manager->requested_mode == READ) {
        DP("ERROR: RegistryManager is in read-only mode @registry_manager_update_at()\n");
        return;
    }

    if (reg_header_get_next_RRN(manager->header) <= RRN) return;

    _seek_registry(manager, RRN);

    if (_update_current_registry(manager, new_data) == true) { //Indica que o registro a ser atualizado não era deletado e não houveram mais erros
        reg_header_set_updated_count(manager->header, H_INCREASE);
    }
}   

void registry_manager_for_each(RegistryManager *manager, RMForeachCallback callback_func) {
    registry_manager_for_each_match(manager, NULL, callback_func);
}

bool registry_manager_is_empty(RegistryManager *manager) {
	if (manager == NULL || manager->bin_file == NULL) {
		DP("WARNING: trying to check whether a registrymanager that is not opened is empty or not\n");
		return false;
	}

    return reg_header_get_registries_count(manager->header) == 0;
}

RegistryHeader *registry_manager_get_registry_header (RegistryManager *manager) {
    if (manager == NULL) {
        return NULL;
    }

    return manager->header;
}