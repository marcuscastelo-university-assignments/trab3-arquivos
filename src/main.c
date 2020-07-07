/*
    Trabalho 2 de SCC0215 - Organização de Arquivos

    Integrantes do grupo:
        - Lucas de Medeiros França Romero             nUSP: 11219154
        - Marcus Vinicius Castelo Branco Martins      nUSP: 11219237
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include "data_manager.h"
#include "b_tree_manager.h"

#include "csv_reader.h"

#include "registry.h"
#include "registry_linked_list.h"

#include "string_utils.h"
#include "bool.h"

#define print_erro(x) fprintf(stderr, x);

typedef struct _Funcionalidade6ExtensionInfo
{
    FILE *b_tree_file_stream;
    int idNascimento, RRN;
    void (*callback)(struct _Funcionalidade6ExtensionInfo *info);
} Funcionalidade6ExtensionInfo;


/**
 *  Função de exibição de erro padronizada de acordo com as especificações
 *  do trabalho e o resultado da operação de abertura do data_manager.
 * 
 *  Parâmetros:
 *      OPEN_RESULT open_result -> enum retornada pela função data_manager_open(), 
 *          indicando o resultado da operação.
 *  Retorno:
 *      void
 */
void print_data_manager_open_result_message(OPEN_RESULT open_result) {
    if (open_result & (OPEN_FAILED | OPEN_INCONSISTENT)) printf("Falha no processamento do arquivo.\n");
    else if (open_result & OPEN_EMPTY) printf("Registro Inexistente.\n"); //(obs: ignorando especificação do trabalho 1, que usa 'i' minúsculo. Supondo ser um erro de digitação)
    else if (open_result & OPEN_INVALID_ARGUMENT) printf("ERRO: Um argumento inválido foi informado\n");
}

/**
 *  Funcionalidade 1: Gerar arquivo binário a partir de CSV
 *  Parâmetros:
 *      const char *csv_filename -> nome do arquivo csv do qual serão lidos os registros
 *      const char *bin_filename -> nome do arquivo binário em que serão escritos os registros
 *  Retorno: void
 */
void funcionalidade1(char *csv_filename, char *bin_filename) {
    if (bin_filename == NULL) {
        print_erro("ERROR: invalid filename @funcionalidade1()\n");
        return;
    }
    
    //Cria um DataManager para fazer o gerenciamento dos dados, ler especificação do TAD
    DataManager *data_manager = data_manager_create(bin_filename);

    if (data_manager == NULL) {
        print_erro("ERROR: couldn't create DataManager @funcionalidade1()\n");
        return;
    }

    //Lê o csv e coloca os dados em uma estrutura de dados
    VirtualRegistryArray *registries = csv_read_all_lines(csv_filename);
    
    if (registries == NULL) {
        printf("Falha no carregamento do arquivo.\n");
        return;
    }

    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = data_manager_open(data_manager, WRITE);
    if (open_result != OPEN_OK) {
        data_manager_delete(&data_manager);
        print_data_manager_open_result_message(open_result);
        return;
    }

    //Se não houverem registros no csv, encerra o programa
    if (registries->data_arr == NULL) {
        data_manager_delete(&data_manager);
        virtual_registry_array_delete(&registries);
        binarioNaTela(bin_filename);
        return;
    } 
    
    //Escreve os registros do csv no arquivo binário
    data_manager_insert_arr_at_end(data_manager, registries->data_arr, registries->size);

    //Libera memória da estrutura de dados utilizada para conter o csv
    virtual_registry_array_delete(&registries);

    //Define o status como "1", fecha o arquivo e desaloca a memoria
    data_manager_delete(&data_manager);

    binarioNaTela(bin_filename);
}

void _DMForeachCallback_print_register(DataManager *manager, VirtualRegistry *registry) {
    virtual_registry_print(registry);
}

/* 
 *  Funcionalidade 2: Abrir arquivo binário já existente
 *  Parâmetros:
 *      const char *bin_filename -> nome do arquivo em que serão lidos os registros
 *  Retorno: void      
 */
void funcionalidade2(char *bin_filename) {
    if (bin_filename == NULL) {
        fprintf(stderr, "ERROR: invalid filename @funcionalidade2()\n");
        return;
    }

    //Cria um DataManager para fazer o gerenciamento dos dados
    DataManager *data_manager = data_manager_create(bin_filename);
    
    if (data_manager == NULL) {
        print_erro("ERROR: couldn't allocate memory for data_manager @funcionalidade2()\n");
        return;
    }

    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = data_manager_open(data_manager, READ);
    if (open_result != OPEN_OK) {
        data_manager_delete(&data_manager);
        print_data_manager_open_result_message(open_result);
        return;
    }

    //Se não houverem registros, exibir mensagem conforme especificação do trabalho (obs: ignorando especificação do trabalho 1, que usa 'i' minúsculo. Supondo ser um erro de digitação)
    if (data_manager_is_empty(data_manager)) printf("Registro Inexistente.\n");
    else data_manager_for_each(data_manager, _DMForeachCallback_print_register);

    //Deleta o DataManager, fechando o arquivo e liberando a memoria
    data_manager_delete(&data_manager);
}


/**
 *  Funcionalidade 3: Exbibe todos os registros condizentes com o filtro dado pelo usuário
 *  Parâmetros:
 *      char *bin_filename -> nome do arquivo em que serão lidos os registros
 *  Retorno: void
 */
void funcionalidade3 (char *bin_filename) {
    if (bin_filename == NULL) {
        print_erro("ERROR: (parameter) invalid null filename @funcionalidade3()\n");
        return;
    }

    //Cria uma variavel do tipo DataManager para fazer o gerenciamento dos dados
    DataManager *data_manager = data_manager_create(bin_filename);

    if (data_manager == NULL) {
        print_erro("ERROR: couldn't create DataManager @funcionalidade3()\n");
        return;
    }
    
    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = data_manager_open(data_manager, READ);
    if (open_result != OPEN_OK) {
        print_data_manager_open_result_message(open_result);

        //Ao interromper o fluxo padrão, é necessário limpar a memória
        data_manager_delete(&data_manager);
        return;
    }

    //Le os campos e valores dados pelo usuarios para ser usado na busca
    VirtualRegistry *reg_search_terms = virtual_registry_create_from_input(false);
    if (reg_search_terms == NULL) {
        print_erro("ERROR: couldn't get registry filters from user @funcionalidade3\n");
        return;
    }

    //Procura no arquivo os registros que correspondam ao filtro do usuario e retorna no formato de VirtualRegistryArray*
    VirtualRegistryArray *reg_arr = data_manager_fetch(data_manager, reg_search_terms);
    if (reg_arr == NULL) {
        fprintf(stderr, "ERROR: couldn't fetch data @funcionalidade3()\n");
        return;
    }

    int size = reg_arr->size;

    //Se não houver registros, exibe mensagem conforme especificação do trabalho
    if (size == 0) printf("Registro Inexistente.\n");

    //Printa todos os registros, se houver algum
    for (int i = 0; i < size; i++) virtual_registry_print(reg_arr->data_arr[i]);
    
    //Desaloca toda a memoria utilizada e fecha o arquivo (efeito colateral de deletar o DataManager)
    virtual_registry_array_delete(&reg_arr);
    virtual_registry_delete(&reg_search_terms);
    data_manager_delete(&data_manager);
}

/*
    Funcionalidade 4: Lê um registro em um dado RRN
    Parâmetros:
        char *bin_filename -> nome do arquivo binario que tera' seu registro lido
        char *RRN_str -> string contendo o RRN (int) do registro a ser lido
    Retorno: void
*/
void funcionalidade4 (char *bin_filename, char *RRN_str) {
    if (bin_filename == NULL) {
        print_erro("ERROR: invalid filename @funcionalidade4()\n");
        return;
    }

    if (RRN_str == NULL) {
        print_erro("ERROR: invalid RRN @funcionalidade5()\n");
        return;
    }

    //Cria uma variavel do tipo DataManager para fazer o gerenciamento dos dados    
    DataManager *data_manager = data_manager_create(bin_filename);
    int RRN = atoi(RRN_str);

    if (data_manager == NULL) {
        print_erro("ERROR: couldn't allocate memory for data_manager @funcionalidade4()\n");
        return;
    }
    
    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = data_manager_open(data_manager, READ);
    if (open_result != OPEN_OK) {
        data_manager_delete(&data_manager);
        print_data_manager_open_result_message(open_result);
        return;
    }

    //pega o registro no RRN dado
    VirtualRegistry *reg_data = data_manager_fetch_at(data_manager, RRN);


    //Printa o registro caso exista
    if (reg_data == NULL)
        printf("Registro Inexistente.\n");
    else
        virtual_registry_print(reg_data);

    //Desaloca toda a memoria
    if (reg_data != NULL)
        virtual_registry_delete(&reg_data);
    
    data_manager_delete(&data_manager);
}


/*
    Funcionalidade 5: deleta registros baseados em filtros dados pelo usuario
    Parâmetros:
        char *bin_filename -> nome do arquivo binario
        char *n_str -> string contendo a quantidade (int) de filtros
    Retorno: void
*/
void funcionalidade5 (char *bin_filename, char *n_str) {
    //Validação de parâmetros
    if (bin_filename == NULL) {
        print_erro("ERROR: invalid filename @funcionalidade5()\n");
        return;
    }

    if (n_str == NULL) {
        print_erro("ERROR: invalid parameter @funcionalidade5()\n");
        return;
    }

    //Cria uma variável do tipo DataManager para fazer o gerenciamento dos dados     
    DataManager *data_manager = data_manager_create(bin_filename);
    int n = atoi(n_str);

    if (data_manager == NULL) {
        print_erro("ERROR: couldn't allocate memory for data_manager @funcionalidade5()\n");
        return;
    }
    
    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = data_manager_open(data_manager, MODIFY);

    if (open_result == OPEN_EMPTY) {
        data_manager_delete(&data_manager);
        binarioNaTela(bin_filename);
        return;
    }

    if (open_result != OPEN_OK && open_result != OPEN_EMPTY) { //De acordo com os casos de teste, o arquivo vazio deve ser tratado sem mensagem de erro para essa funcionalidade
        print_data_manager_open_result_message(open_result);
        data_manager_delete(&data_manager);
        return;
    }

    RegistryLinkedList *list = registry_linked_list_create();
    VirtualRegistryArray *reg_arr;
    VirtualRegistryFilter *reg_filter;  
    
    if (list == NULL) {
        print_erro("ERROR: couldn't allocate memory for RegistryLinkedList @funcionalidade5()\n");
        data_manager_delete(&data_manager);
        return;
    }

    //Le todos os filtros de remocao dados pelo usuario e insere na linked list
    for (int i = 0; i < n; i++) {
        //Cria um filtro (false indica que o registro deve ser interpretado como filtro)
        reg_filter = virtual_registry_create_from_input(false); //ler documentação de virtual_registry_create_from_input()
        if (reg_filter == NULL) {
            print_erro("ERROR: couldn't allocate memory for VirtualRegistry @funcionalidade5()\n");
            data_manager_delete(&data_manager);
            registry_linked_list_delete(&list, true);
            return;
        }
        
        registry_linked_list_insert(list, reg_filter);
    }

    //transforma a linked list em um array e desaloca memoria
    //sem desalocar a memoria dos registros da linked list, pois
    //sao usados no array
    reg_arr = registry_linked_list_to_array(list);
    if (reg_arr == NULL) {
        //A mensagem de erro é um pouco genérica pois, nas funções interiores, o erro é especificado
        print_erro("ERRO: Failed to convert linked list to RegistryDataArray @funcionalidade5()\n");
        data_manager_delete(&data_manager);
        registry_linked_list_delete(&list, true);
        return;
    }

    //Libera a memória da lista lidada sem apagar os seus itens pelo mesmo motivo acima
    registry_linked_list_delete(&list, false);

    //Remove os registros que contiverem as informações especificadas (remove os que derem match)
    data_manager_remove_matches(data_manager, reg_arr);

    //desaloca toda a memoria, fecha o arquivo e seta o status como consistente
    virtual_registry_array_delete(&reg_arr);
    data_manager_delete(&data_manager);

    binarioNaTela(bin_filename);
}

/*
    Funcionalidade 6: Inserir novo registro, ignorando espaços livres por deleção (abordagem de deleção estática)
    Parâmetros:
        char *bin_filename -> nome do arquivo binario
        char *n_str -> string contendo a quantidade (int) de registros a serem inseridos
    Retorno: void
*/

void funcionalidade6 (char *bin_filename, char *n_str, Funcionalidade6ExtensionInfo *extInfo) {
    //Validação de parâmetros
    if (bin_filename == NULL) {
        print_erro("ERROR: invalid filename @funcionalidade6()\n");
        return;
    }
    
    if (n_str == NULL) {
        print_erro("ERROR: invalid parameter @funcionalidade6()\n");
        return;
    }

    //Tenta criar um DataManager
    DataManager *data_manager = data_manager_create(bin_filename);
    if (data_manager == NULL) {
        fprintf(stderr, "ERRO: unable to create DataManager @funcionalidade6\n");
        return;
    }

    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = data_manager_open(data_manager, MODIFY);
    if (open_result != OPEN_OK && open_result != OPEN_EMPTY) {
        data_manager_delete(&data_manager);
        print_data_manager_open_result_message(open_result);
        return;
    }

    char *campos[] = {"cidadeMae","cidadeBebe","idNascimento","idadeMae","dataNascimento","sexoBebe","estadoMae","estadoBebe"}; 

    int n = atoi(n_str);
    VirtualRegistry *reg_data;
    char *value_tmp = NULL;
    for (int i = 0; i < n; i++)
    {
        reg_data = virtual_registry_create();
        for (int j = 0; j < sizeof(campos)/sizeof(char*); j++)
        {

            value_tmp = virtual_registry_read_value_from_input(campos[j]);
            virtual_registry_set_field(reg_data, campos[j], value_tmp);
            free(value_tmp);

        }
        int insertedRRN = data_manager_insert_at_end(data_manager, reg_data);
        if (extInfo != NULL && extInfo->callback != NULL) {
            extInfo->idNascimento = reg_data->idNascimento;
            extInfo->RRN = insertedRRN;
            extInfo->callback(extInfo);
        }
    }

    data_manager_delete(&data_manager);
    binarioNaTela(bin_filename);
}

/*
    Funcionalidade 7: Atualizar um registro pelo seu RRN
    Parâmetros:
        char *bin_filename -> nome do arquivo binario
        char *n_str -> string contendo a quantidade (int) de registros a serem atualizados
    Retorno: void
*/
void funcionalidade7 (char *bin_filename, char *n_str) {
    //Validação de parâmetros
    if (bin_filename == NULL) {
        print_erro("ERROR: invalid filename @funcionalidade7()\n");
        return;
    }
    
    if (n_str == NULL) {
        print_erro("ERROR: invalid parameter @funcionalidade7()\n");
        return;
    }

    //Tenta criar o DataManager
    DataManager *data_manager = data_manager_create(bin_filename);
    if (data_manager == NULL) {
        fprintf(stderr, "ERRO: unable to create DataManager @funcionalidade7\n");
        return;
    }

    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = data_manager_open(data_manager, MODIFY);
    if (open_result != OPEN_OK) {
        data_manager_delete(&data_manager);
        print_data_manager_open_result_message(open_result);
        return;
    }

    int n = atoi(n_str);

    VirtualRegistryUpdater *reg_updater = NULL;
    int RRN;
    //Atualiza n registros
    for (int i = 0; i < n; i++) {
        scanf("%d", &RRN);

        //Cria um registro do tipo updater (registro com mascara de bits)
        reg_updater = virtual_registry_create_from_input(false); //O false indica que o registro possui mascara de bits (ler documentação de virtual_registry_create_from_input())
        if (reg_updater == NULL) {
            fprintf(stderr, "ERROR: failed to create VirtualRegistry from input @funcionalidade7()\n");
            break;
        }
        //Atualiza no disco
        data_manager_update_at(data_manager, RRN, reg_updater);
        
        //Libera a memória do updater
        virtual_registry_delete(&reg_updater);
    }

    //Libera o DataManager e fecha o arquivo
    data_manager_delete(&data_manager);

    binarioNaTela(bin_filename);
}

//TODO: comment
void funcionalidade8 (char *bin_filename, char *b_tree_filename) {
    //Validação de parâmetros
    if (bin_filename == NULL) {
        print_erro("ERROR: invalid filename @funcionalidade8()\n");
        return;
    }
    
    if (b_tree_filename == NULL) {
        print_erro("ERROR: invalid parameter @funcionalidade8()\n");
        return;
    }
}

void funcionalidade9 (char *bin_filename, char *n_str, char *field, char *value) {

}

void funcionalidade10 (Funcionalidade6ExtensionInfo *info) {
    FILE *stream = info->b_tree_file_stream;
    BTHeader *header = b_tree_header_create();
    b_tree_header_read_from_bin(header, stream);
    // BTreeManager *manager = b_tree_manager_create(stream, header);

    //TODO: Inserir
    // b_tree_manager_write_at(manager, info->RRN, );
}


/**
 *  Inicializa um vetor de parâmetros lidos do stdin
 *  Parâmetros:
 *      int quantity -> quantidade de parâmetros a serem lidos (tamanho do vetor)
 *  Retorno:
 *      char ** -> vetor com 'quantity' parâmetros
 */
char **init_params(int quantity){ 

    char **params = malloc(sizeof(char*) * quantity);

    if (params == NULL) {
        fprintf(stderr, "ERROR: not enough memory @init_params!\n");
        exit(1);
        return NULL;
    }

    //Lê do stdin todos os parâmetros, alimentando o vetor
    for (int i = 0; i < quantity; i++) {
        params[i] = NULL;
        scanf("%ms", &params[i]);
    }
    
    return params;
}

/**
 *  Libera a memória usada pelos parâmetros, bem como a usada pelo vetor que os guarda
 *  Parâmetros: 
 *      char *** params_ptr -> referência ao vetor dinâmico que guarda os parâmetros
 *      int quantity -> quantidade de elementos no vetor
 *  Retorno: void   
 */
void free_params(char ***params_ptr, int quantity) {
    #define params (*params_ptr)

    if (params_ptr == NULL) return;

    //Libera todos os char* do vetor
    for (int i = 0; i < quantity; i++) free(params[i]);

    //Libera o char**
    free(params);
    params = NULL;    

    #undef params
}


/**
 *  Função principal: responsável por guiar o fluxo do programa separado por funcionalidades
 *  Lê um caractere representando a funcionalidade. Depois, lê n parâmetros e passa eles para a funcionalidade especificada
 *  OBS: n é definido de acordo com a funcionalidade
 *  Retorno: int - código de erro do programa
 */
int main(void) {   
    //Entrada esperada para o programa: <funcionalidade_code> param1,[param2,param3...]

    //Código da funcionalidade desejada
    int funcionalidade_code;

    //Parâmetros, são inicializados dentro do switch por serem de tamanho variável
    char **params = NULL;

    //Lê o código de funcionalidade
    scanf("%d", &funcionalidade_code);

    //Decide qual função usar baseado na funcionalidade escolhida
    switch (funcionalidade_code) {
        //Para cada funcionalidade: lê os n parâmetros e chama a função com estes.
        case 1:
            params = init_params(2); //n = 2
            funcionalidade1(params[0], params[1]);
            free_params(&params, 2);
            break;
            
        case 2:
            params = init_params(1); //n = 1
            funcionalidade2(params[0]);
            free_params(&params, 1);
            break;
            
        case 3:
            params = init_params(1);
            funcionalidade3(params[0]);
            free_params(&params, 1);
            break;

        case 4:
            params = init_params(2);
            funcionalidade4(params[0], params[1]);
            free_params(&params, 2);
            break;

        case 5:
            params = init_params(2);
            funcionalidade5(params[0], params[1]);
            free_params(&params, 2);
            break;

        case 6:
            params = init_params(2);
            funcionalidade6(params[0], params[1], NULL);
            free_params(&params, 2);
            break;
        
        case 7:
            params = init_params(2);
            funcionalidade7(params[0], params[1]);
            free_params(&params, 2);
            break;

        case 8:
            params = init_params(2);
            funcionalidade8(params[0], params[1]);
            free_params(&params, 2);
            break;

        case 9:
            params = init_params(4);
            funcionalidade9(params[0], params[1], params[2], params[3]);
            free_params(&params, 4);
            break;

        case 10:
            params = init_params(2);
            Funcionalidade6ExtensionInfo extensionInfo;
            extensionInfo.b_tree_file_stream = fopen("foda", "rb+");
            extensionInfo.callback = funcionalidade10;
            funcionalidade6(params[0], params[1], &extensionInfo);

            fclose(extensionInfo.b_tree_file_stream);
            free_params(&params, 2);
            break;

        default:
            printf("Funcionalidade %c não implementada.\n", funcionalidade_code);
            break;
    }

    return 0;
}
