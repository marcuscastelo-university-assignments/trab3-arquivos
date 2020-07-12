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

#include "registry_manager.h"
#include "b_tree_manager.h"
#include "b_tree_node.h"

#include "csv_reader.h"

#include "registry.h"
#include "registry_linked_list.h"
#include "registry_header.h"

#include "string_utils.h"
#include "bool.h"
#include "debug.h"

typedef struct _Funcionalidade6ExtensionInfo
{
    BTreeManager *btman;
    int idNascimento, RRN;
    void (*callback)(struct _Funcionalidade6ExtensionInfo *info);
} Funcionalidade6ExtensionInfo;


/**
 *  Função de exibição de erro padronizada de acordo com as especificações
 *  do trabalho e o resultado da operação de abertura do registry_manager.
 * 
 *  Parâmetros:
 *      OPEN_RESULT open_result -> enum retornada pela função registry_manager_open(), 
 *          indicando o resultado da operação.
 *  Retorno:
 *      void
 */
static void _print_open_result_message(OPEN_RESULT open_result) {
    if (open_result & (OPEN_FAILED | OPEN_INCONSISTENT)) printf("Falha no processamento do arquivo.\n");
    else if (open_result & OPEN_INVALID_ARGUMENT) DP("ERRO: Um argumento inválido foi informado\n");
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
        DP("ERROR: invalid filename @funcionalidade1()\n");
        return;
    }
    
    //Cria um RegistryManager para fazer o gerenciamento dos dados, ler especificação do TAD
    RegistryManager *registry_manager = registry_manager_create();

    if (registry_manager == NULL) {
        DP("ERROR: couldn't create RegistryManager @funcionalidade1()\n");
        return;
    }

    //TODO: csv ler e ir salvando
    //Lê o csv e coloca os dados em uma estrutura de dados
    VirtualRegistryArray *registries = csv_read_all_lines(csv_filename);
    
    if (registries == NULL) {
        printf("Falha no carregamento do arquivo.\n");
        return;
    }

    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, CREATE);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        _print_open_result_message(open_result);
        return;
    }

    //Se não houverem registros no csv, encerra o programa
    if (registries->data_arr == NULL) {
        registry_manager_free(&registry_manager);
        virtual_registry_array_delete(&registries);
        binarioNaTela(bin_filename);
        return;
    } 
    
    //Escreve os registros do csv no arquivo binário
    registry_manager_insert_arr_at_end(registry_manager, registries->data_arr, registries->size);

    //Libera memória da estrutura de dados utilizada para conter o csv
    virtual_registry_array_delete(&registries);

    //Define o status como "1", fecha o arquivo e desaloca a memoria
    registry_manager_free(&registry_manager);

    binarioNaTela(bin_filename);
}

void _DMForeachCallback_print_register(RegistryManager *manager, VirtualRegistry *registry) {
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
        DP("ERROR: invalid filename @funcionalidade2()\n");
        return;
    }

    //Cria um RegistryManager para fazer o gerenciamento dos dados
    RegistryManager *registry_manager = registry_manager_create();
    
    if (registry_manager == NULL) {
        DP("ERROR: couldn't allocate memory for registry_manager @funcionalidade2()\n");
        return;
    }

    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, READ);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        _print_open_result_message(open_result);
        return;
    }

    //Se não houverem registros, exibir mensagem conforme especificação do trabalho (obs: ignorando especificação do trabalho 1, que usa 'i' minúsculo. Supondo ser um erro de digitação)
    if (registry_manager_is_empty(registry_manager)) printf("Registro Inexistente.\n");
    else registry_manager_for_each(registry_manager, _DMForeachCallback_print_register);

    //Deleta o RegistryManager, fechando o arquivo e liberando a memoria
    registry_manager_free(&registry_manager);
}


/**
 *  Funcionalidade 3: Exbibe todos os registros condizentes com o filtro dado pelo usuário
 *  Parâmetros:
 *      char *bin_filename -> nome do arquivo em que serão lidos os registros
 *  Retorno: void
 */
void funcionalidade3 (char *bin_filename) {
    if (bin_filename == NULL) {
        DP("ERROR: (parameter) invalid null filename @funcionalidade3()\n");
        return;
    }

    //Cria uma variavel do tipo RegistryManager para fazer o gerenciamento dos dados
    RegistryManager *registry_manager = registry_manager_create();

    if (registry_manager == NULL) {
        DP("ERROR: couldn't create RegistryManager @funcionalidade3()\n");
        return;
    }
    
    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, READ);
    if (open_result != OPEN_OK) {
        _print_open_result_message(open_result);

        //Ao interromper o fluxo padrão, é necessário limpar a memória
        registry_manager_free(&registry_manager);
        return;
    }

    //Le os campos e valores dados pelo usuarios para ser usado na busca
    VirtualRegistry *search_sample_registry = virtual_registry_create_from_input(false);
    if (search_sample_registry == NULL) {
        DP("ERROR: couldn't get registry filters from user @funcionalidade3\n");
        return;
    }

    VirtualRegistryArray *reg_search_terms = virtual_registry_array_create_unique(search_sample_registry);
    if (reg_search_terms == NULL) {
        DP("ERROR: couldn't allocate memory for VirtualRegistryArray @funcionalidade3\n");
        return;
    }

    int foundRegistersCount = registry_manager_for_each_match(registry_manager, reg_search_terms, _DMForeachCallback_print_register);

    //Se não houver registros, exibe mensagem conforme especificação do trabalho
    if (foundRegistersCount == 0) printf("Registro Inexistente.\n");

    //Desaloca toda a memoria utilizada e fecha o arquivo (efeito colateral de deletar o RegistryManager)
    virtual_registry_array_delete(&reg_search_terms);
    registry_manager_free(&registry_manager);
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
        DP("ERROR: invalid filename @funcionalidade4()\n");
        return;
    }

    if (RRN_str == NULL) {
        DP("ERROR: invalid RRN @funcionalidade5()\n");
        return;
    }

    //Cria uma variavel do tipo RegistryManager para fazer o gerenciamento dos dados    
    RegistryManager *registry_manager = registry_manager_create();
    int RRN = atoi(RRN_str);

    if (registry_manager == NULL) {
        DP("ERROR: couldn't allocate memory for registry_manager @funcionalidade4()\n");
        return;
    }
    
    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, READ);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        _print_open_result_message(open_result);
        return;
    }

    if (registry_manager_is_empty(registry_manager)) printf("Registro Inexistente.\n");

    //pega o registro no RRN dado
    VirtualRegistry *reg_data = registry_manager_fetch_at(registry_manager, RRN);


    //Printa o registro caso exista
    if (reg_data == NULL)
        printf("Registro Inexistente.\n");
    else
        virtual_registry_print(reg_data);

    //Desaloca toda a memoria
    if (reg_data != NULL)
        virtual_registry_free(&reg_data);
    
    registry_manager_free(&registry_manager);
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
        DP("ERROR: invalid filename @funcionalidade5()\n");
        return;
    }

    if (n_str == NULL) {
        DP("ERROR: invalid parameter @funcionalidade5()\n");
        return;
    }

    //Cria uma variável do tipo RegistryManager para fazer o gerenciamento dos dados     
    RegistryManager *registry_manager = registry_manager_create();
    int n = atoi(n_str);

    if (registry_manager == NULL) {
        DP("ERROR: couldn't allocate memory for registry_manager @funcionalidade5()\n");
        return;
    }
    
    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, MODIFY);

    if (open_result != OPEN_OK) { 
        _print_open_result_message(open_result);
        registry_manager_free(&registry_manager);
        return;
    }

    if (registry_manager_is_empty(registry_manager)) {
        registry_manager_free(&registry_manager);
        binarioNaTela(bin_filename);
        return;
    }

    RegistryLinkedList *list = registry_linked_list_create();
    VirtualRegistryArray *reg_arr;
    VirtualRegistryFilter *reg_filter;  
    
    if (list == NULL) {
        DP("ERROR: couldn't allocate memory for RegistryLinkedList @funcionalidade5()\n");
        registry_manager_free(&registry_manager);
        return;
    }

    //Le todos os filtros de remocao dados pelo usuario e insere na linked list
    for (int i = 0; i < n; i++) {
        //Cria um filtro (false indica que o registro deve ser interpretado como filtro)
        reg_filter = virtual_registry_create_from_input(false); //ler documentação de virtual_registry_create_from_input()
        if (reg_filter == NULL) {
            DP("ERROR: couldn't allocate memory for VirtualRegistry @funcionalidade5()\n");
            registry_manager_free(&registry_manager);
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
        DP("ERRO: Failed to convert linked list to RegistryDataArray @funcionalidade5()\n");
        registry_manager_free(&registry_manager);
        registry_linked_list_delete(&list, true);
        return;
    }

    //Libera a memória da lista lidada sem apagar os seus itens pelo mesmo motivo acima
    registry_linked_list_delete(&list, false);

    //Remove os registros que contiverem as informações especificadas (remove os que derem match)
    registry_manager_remove_matches(registry_manager, reg_arr);

    //desaloca toda a memoria, fecha o arquivo e seta o status como consistente
    virtual_registry_array_delete(&reg_arr);
    registry_manager_free(&registry_manager);

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
        DP("ERROR: invalid filename @funcionalidade6()\n");
        return;
    }
    
    if (n_str == NULL) {
        DP("ERROR: invalid parameter @funcionalidade6()\n");
        return;
    }

    //Tenta criar um RegistryManager
    RegistryManager *registry_manager = registry_manager_create();
    if (registry_manager == NULL) {
        DP("ERRO: unable to create RegistryManager @funcionalidade6\n");
        return;
    }

    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, MODIFY);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        _print_open_result_message(open_result);
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
        int insertedRRN = registry_manager_insert_at_end(registry_manager, reg_data);
        if (extInfo != NULL && extInfo->callback != NULL) {
            extInfo->idNascimento = reg_data->idNascimento;
            extInfo->RRN = insertedRRN;
            extInfo->callback(extInfo);
        }
    }

    registry_manager_free(&registry_manager);
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
        DP("ERROR: invalid filename @funcionalidade7()\n");
        return;
    }
    
    if (n_str == NULL) {
        DP("ERROR: invalid parameter @funcionalidade7()\n");
        return;
    }

    //Tenta criar o RegistryManager
    RegistryManager *registry_manager = registry_manager_create();
    if (registry_manager == NULL) {
        DP("ERRO: unable to create RegistryManager @funcionalidade7\n");
        return;
    }


    //TODO: ver no runcodes se precisa de "Registro Inexistente"
    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, MODIFY);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        _print_open_result_message(open_result);
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
            DP("ERROR: failed to create VirtualRegistry from input @funcionalidade7()\n");
            break;
        }
        //Atualiza no disco
        registry_manager_update_at(registry_manager, RRN, reg_updater);
        
        //Libera a memória do updater
        virtual_registry_free(&reg_updater);
    }

    //Libera o RegistryManager e fecha o arquivo
    registry_manager_free(&registry_manager);

    binarioNaTela(bin_filename);
}

//TODO: remove debug function
void printint (int a) {printf("%d\n", a);}

//TODO: comment
void funcionalidade8 (char *reg_bin_filename, char *b_tree_filename) {
    //Validação de parâmetros
    if (reg_bin_filename == NULL || b_tree_filename == NULL) {
        DP("ERROR: invalid filename @funcionalidade8()\n");
        return;
    }

    //Cria um RegistryManager para ler todos os registros em disco
    RegistryManager *registry_manager = registry_manager_create();

    if (registry_manager == NULL) {
        DP("ERROR: couldn't create RegistryManager @funcionalidade8()\n");
        return;
    }

    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, reg_bin_filename, READ);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        _print_open_result_message(open_result);
        return;
    }

    //TODO: fclose
    BTreeManager *b_tree_manager = b_tree_manager_create();

    if (b_tree_manager == NULL) {
        DP("ERROR: couldn't create BTreeManager @funcionalidade8()\n");
        return;
    }

    open_result = b_tree_manager_open(b_tree_manager, b_tree_filename, CREATE);

    if (open_result != OPEN_OK) {
        _print_open_result_message(open_result);
        b_tree_manager_free(&b_tree_manager);
        return;
    }

    RegistryHeader *reg_header = registry_manager_get_registry_header(registry_manager);
    int registryCount = reg_header_get_registries_count(reg_header);
    int RRN = -1;
    
    for (int i = 0; i < registryCount; i++) {
        RRN++;
        VirtualRegistry *reg = registry_manager_fetch_at(registry_manager, RRN);
        if (reg == NULL) {
            i--;
            continue;
        }

        b_tree_manager_insert(b_tree_manager, reg->idNascimento, RRN);
        virtual_registry_free(&reg);
    }

    b_tree_manager_free(&b_tree_manager);
    registry_manager_free(&registry_manager);

    binarioNaTela(b_tree_filename);
    return;
}

//TODO: remover includes desnecessários nos arquivos
void funcionalidade9 (char *reg_filename, char *b_tree_filename, char *searchFieldStr, char *searchValueStr) {
    if (reg_filename == NULL || b_tree_filename == NULL || searchFieldStr == NULL || searchValueStr == NULL) {
        DP("Invalid arguments @funcionalidade9()\n");
        return;
    }

    if (strcmp(searchFieldStr,"idNascimento") != 0) {
        DP("ERROR: trying to search in b-tree by unsuported field @funcionalidade9()\n");
        printf("Falha no processamento do arquivo\n");
        return;
    }

    BTreeManager *btman = b_tree_manager_create();
    if (btman == NULL) {
        DP("ERROR: couldn't allocate memory for BTreeManager\n");
        return;
    }

    RegistryManager *regman = registry_manager_create();
    if (regman == NULL) {
        DP("ERROR: couldn't allocate memory for RegistryManager\n");
        return;
    }

    OPEN_RESULT o_res;
    o_res = b_tree_manager_open(btman, b_tree_filename, READ);
    if (o_res != OPEN_OK) {
        _print_open_result_message(o_res);
        b_tree_manager_free(&btman);
        registry_manager_free(&regman);
        return;
    }

    int idNascimento = atoi(searchValueStr);

    //Em caso de resultado NaN
    if (searchValueStr[0] != '0' && idNascimento == 0) {
        DP("Trying to search a non-int idNascimento (idNascimentoStr = '%s')\n", searchValueStr);
        if (DEBUG) return;
    }

    int REG_RRN = b_tree_manager_search_for(btman, idNascimento);

    //Se não for encontrado
    if (REG_RRN == -1) {
        printf("Registro inexistente.\n");
        b_tree_manager_free(&btman);
        registry_manager_free(&regman);
        return;
    }

    o_res = registry_manager_open(regman, reg_filename, READ);
    if (o_res != OPEN_OK) {
        _print_open_result_message(o_res);
        b_tree_manager_free(&btman);
        registry_manager_free(&regman);
        return;
    }

    VirtualRegistry *registry = registry_manager_fetch_at(regman, REG_RRN);

    //Caso de ter sido deletado (ou RRN inválido)
    if (registry == NULL) {
        //TODO: definir comportamento e liberar memória
        DP("Registro deletado não implementado!\n");
        // return;
    }

    //Exibe o registro
    virtual_registry_print(registry);
    virtual_registry_free(&registry);    

    //TODO: mostrar a quantidade de páginas de disco que foram acessadas : LUCAS

    //TODO: ver se faltou algo

}

void funcionalidade10callback (Funcionalidade6ExtensionInfo *info) {
    b_tree_manager_insert(info->btman, info->idNascimento, info->RRN);
}

void funcionalidade10(char *reg_filename, char *b_tree_filename, char *n_str) {
    BTreeManager *btman = b_tree_manager_create();
    if (btman == NULL) {
        DP("ERROR: couldn't allocate memory for BTreeManager\n");
        return;
    }

    OPEN_RESULT o_res;
    o_res = b_tree_manager_open(btman, b_tree_filename, READ);
    if (o_res != OPEN_OK) {
        _print_open_result_message(o_res);
        b_tree_manager_free(&btman);
    }

    //Informações que são passadas para um callback da funcionalidade 6, que chama a funcionalidade10callback a cada inserção
    Funcionalidade6ExtensionInfo extensionInfo;
    extensionInfo.btman = btman;
    extensionInfo.callback = funcionalidade10callback;
    funcionalidade6(reg_filename, n_str, &extensionInfo);
    b_tree_manager_free(&btman);

    //TODO: checar se o binarioNaTela da func6 deu certo
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
        DP("ERROR: not enough memory @init_params!\n");
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

void teste () {
    BTreeNode *node = b_tree_node_create(1);
    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        b_tree_node_sorted_insert_item(node, i*2, i);
        b_tree_node_insert_P(node, i, 0);
    }
    b_tree_node_insert_P(node, 10, B_TREE_ORDER-1);
    b_tree_node_print(node);
    printf("\n");
    BTreeNode *new = b_tree_node_split_one_to_two(node, 3, 9, 20);
    b_tree_node_print(node);
    printf("\n");
    b_tree_node_print(new);
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
            params = init_params(3);
            funcionalidade10(params[0], params[1], params[2]);
            free_params(&params, 3);
            break;
        case 0:
            teste();
            break;

        default:
            printf("Funcionalidade %c não implementada.\n", funcionalidade_code);
            break;
    }

    return 0;
}
