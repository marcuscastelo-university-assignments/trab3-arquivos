/*
    Trabalho 3 de SCC0215 - Organização de Arquivos

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
#include "pair.h"

/**
 *  Struct usada na funcionalidade10 para informar à funcionalidade6 o que deve ser executado.
 *  Além disso, carrega informações necessárias à execução desse callback * 
 */
typedef struct _Funcionalidade10callbackInfo
{
    //Parâmetros a serem passados ao callback
    BTreeManager *btman;
    int idNascimento, RRN;
    ////

    //Função a ser executada após a inserção de cada registro
    void (*callback)(struct _Funcionalidade10callbackInfo *info);
} Funcionalidade10callbackInfo;

/**
 *  Funcionalidade 1: Gerar arquivo binário a partir de CSV
 *  Parâmetros:
 *      const char *csv_filename -> nome do arquivo csv do qual serão lidos os registros
 *      const char *bin_filename -> nome do arquivo binário em que serão escritos os registros
 *  Retorno: bool -> indica se a funcionalidade foi executada com sucesso.
 */
static bool funcionalidade1(char *csv_filename, char *bin_filename) {
    if (bin_filename == NULL) {
        DP("ERROR: invalid filename @funcionalidade1()\n");
        return false;
    }
    
    //Cria um RegistryManager para fazer a escrita dos registros, ler especificação do TAD
    RegistryManager *registry_manager = registry_manager_create();
    if (registry_manager == NULL) {
        DP("ERROR: couldn't create RegistryManager @funcionalidade1()\n");
        return false;
    }

    //Tenta criar o arquivo de registros, exibindo mensagens de erro de acordo com as especificações se algum erro for encontrado
    OPEN_RESULT o_res = registry_manager_open(registry_manager, bin_filename, CREATE);
    if (o_res != OPEN_OK) {
        registry_manager_free(&registry_manager);
        open_result_print_message(o_res);
        return false;
    }

    //Análogo ao RegistryManager
    CsvReader *csv_reader = csv_reader_create();
    if (registry_manager == NULL) {
        DP("ERROR: couldn't create CsvReader @funcionalidade1()\n");
        return false;
    }
     
    o_res = csv_reader_open(csv_reader, csv_filename);

    //Se o arquivo não possuir headers ou não possuir nenhum registro
    if (o_res != OPEN_OK) {
        registry_manager_free(&registry_manager);
        csv_reader_free(&csv_reader);
        printf("Falha no carregamento do arquivo.\n");
        return false;
    }
    
    //Lê linha a linha do csv, escrevendo-a no binário até que o csv acabe
    VirtualRegistry *registry = NULL;
    while ((registry = csv_reader_readline(csv_reader)) != NULL) {
        registry_manager_insert_at_end(registry_manager, registry);
        virtual_registry_free(&registry);
    }

    //Define o status como "1", fecha o arquivo e desaloca a memoria
    registry_manager_free(&registry_manager);
    csv_reader_free(&csv_reader);

    return true;

}

//Função de callback usada nas funcionalidades 2 e 3. é enviada ao RegistryManager para ser usada como callback de funções internas. Nesse caso o callback simplesmente printa o registro
static void _DMForeachCallback_print_register(RegistryManager *manager, VirtualRegistry *registry) {
    virtual_registry_print(registry);
}

/* 
 *  Funcionalidade 2: Abrir arquivo binário já existente
 *  Parâmetros:
 *      const char *bin_filename -> nome do arquivo do qual serão lidos os registros
 *  Retorno: bool -> indica se a funcionalidade foi executada com sucesso.
 */
static bool funcionalidade2(char *bin_filename) {
    if (bin_filename == NULL) {
        DP("ERROR: invalid filename @funcionalidade2()\n");
        return false;
    }

    //Cria um RegistryManager para fazer a leitura dos registros
    RegistryManager *registry_manager = registry_manager_create();
    
    if (registry_manager == NULL) {
        DP("ERROR: couldn't allocate memory for registry_manager @funcionalidade2()\n");
        return false;
    }

    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, READ);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        open_result_print_message(open_result);
        return false;
    }

    //Se não houverem registros, exibir mensagem conforme especificação do trabalho (obs: ignorando especificação do trabalho 1, que usa 'i' minúsculo. Supondo ser um erro de digitação)
    if (registry_manager_is_empty(registry_manager)) printf("Registro inexistente.\n");

    //Senão, exibe todos os registros (usando o callback definido anteriormente)
    else registry_manager_for_each(registry_manager, _DMForeachCallback_print_register);

    //Deleta o RegistryManager, fechando o arquivo e liberando a memoria
    registry_manager_free(&registry_manager);
    return true;
}

/**
 *  Funcionalidade 3: Exbibe todos os registros condizentes com o filtro dado pelo usuário
 *  Parâmetros:
 *      char *bin_filename -> nome do arquivo em que serão lidos os registros
 *  Retorno: bool -> indica se a funcionalidade foi executada com sucesso.
 */
static bool funcionalidade3 (char *bin_filename) {
    if (bin_filename == NULL) {
        DP("ERROR: (parameter) invalid null filename @funcionalidade3()\n");
        return false;
    }

    //Cria uma variavel do tipo RegistryManager para fazer a leitura dos registros
    RegistryManager *registry_manager = registry_manager_create();

    if (registry_manager == NULL) {
        DP("ERROR: couldn't create RegistryManager @funcionalidade3()\n");
        return false;
    }
    
    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, READ);
    if (open_result != OPEN_OK) {
        open_result_print_message(open_result);

        //Ao interromper o fluxo padrão, é necessário limpar a memória
        registry_manager_free(&registry_manager);
        return false;
    }

    //Lê os campos e valores dados pelo usuarios para ser usado na busca
    VirtualRegistry *search_sample_registry = virtual_registry_create_from_input(false); //false indica que os campos não informados devem ser ignorados
    if (search_sample_registry == NULL) {
        DP("ERROR: couldn't get registry filters from user @funcionalidade3\n");
        return false;
    }

    //Como a função registry_manager_for_each_match recebe um vetor de condições, deve ser criado um vetor unitário
    VirtualRegistryArray *reg_search_terms = virtual_registry_array_create_unique(search_sample_registry);
    if (reg_search_terms == NULL) {
        DP("ERROR: couldn't allocate memory for VirtualRegistryArray @funcionalidade3\n");
        return false;
    }

    //Execute o callback definido anteriormente na main.c para printar todos os registros que satisfizerem a condição informada pelo usuário
    int foundRegistersCount = registry_manager_for_each_match(registry_manager, reg_search_terms, _DMForeachCallback_print_register);

    //Se não houver registros, exibe mensagem conforme especificação do trabalho
    if (foundRegistersCount == 0) printf("Registro Inexistente.\n");

    //Desaloca toda a memoria utilizada e fecha o arquivo (efeito colateral de deletar o RegistryManager)
    virtual_registry_array_delete(&reg_search_terms);
    registry_manager_free(&registry_manager);
    return true;
}

/*
    Funcionalidade 4: Lê um registro em um dado RRN
    Parâmetros:
        char *bin_filename -> nome do arquivo binario que tera' seu registro lido
        char *RRN_str -> string contendo o RRN (int) do registro a ser lido (evitar fscanf infinito se entrar com NaN no %d)
    Retorno: bool -> indica se a funcionalidade foi executada com sucesso.
*/
static bool funcionalidade4 (char *bin_filename, char *RRN_str) {
    if (bin_filename == NULL) {
        DP("ERROR: invalid filename @funcionalidade4()\n");
        return false;
    }

    if (RRN_str == NULL) {
        DP("ERROR: invalid RRN @funcionalidade5()\n");
        return false;
    }

    //Cria uma variavel do tipo RegistryManager para fazer o gerenciamento dos dados    
    RegistryManager *registry_manager = registry_manager_create();

    int RRN = atoi(RRN_str);
    if (RRN_str[0] != '0' && RRN == 0) {
        DP("ERROR: invalid non-int RRN\n");
        return false;
    }

    if (registry_manager == NULL) {
        DP("ERROR: couldn't allocate memory for registry_manager @funcionalidade4()\n");
        return false;
    }
    
    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, READ);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        open_result_print_message(open_result);
        return false;
    }

    //Se o arquivo estiver vazio, nem tenta buscar o RRN 
    if (registry_manager_is_empty(registry_manager)) {
        printf("Registro Inexistente.\n");
        registry_manager_free(&registry_manager);
        return true;
    }

    //pega o registro no RRN dado
    VirtualRegistry *reg_data = registry_manager_fetch_at(registry_manager, RRN);

    //Printa o registro caso exista
    if (reg_data != NULL) {
        virtual_registry_print(reg_data);
        virtual_registry_free(&reg_data);
    }
    else {
        printf("Registro Inexistente.\n");
    }

    registry_manager_free(&registry_manager);
    return true;
}

/*
    Funcionalidade 5: deleta registros baseados em filtros dados pelo usuario
    Parâmetros:
        char *bin_filename -> nome do arquivo binario
        char *n_str -> string contendo a quantidade (int) de filtros
    Retorno: bool -> indica se a funcionalidade foi executada com sucesso.
*/
static bool funcionalidade5 (char *bin_filename, char *n_str) {
    //Validação de parâmetros
    if (bin_filename == NULL) {
        DP("ERROR: invalid filename @funcionalidade5()\n");
        return false;
    }

    if (n_str == NULL) {
        DP("ERROR: invalid parameter @funcionalidade5()\n");
        return false;
    }

    //Cria uma variável do tipo RegistryManager para fazer o gerenciamento dos dados     
    RegistryManager *registry_manager = registry_manager_create();
    int n = atoi(n_str);
    if (n_str[0] != '0' && n == 0) {
        DP("ERROR: invalid non-int n\n");
        return false;
    }

    if (registry_manager == NULL) {
        DP("ERROR: couldn't allocate memory for registry_manager @funcionalidade5()\n");
        return false;
    }
    
    //Abre o arquivo para escrita, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, MODIFY);

    if (open_result != OPEN_OK) { 
        open_result_print_message(open_result);
        registry_manager_free(&registry_manager);
        return false;
    }

    //Se o arquivo estiver vazio, não apague nada
    if (registry_manager_is_empty(registry_manager)) {
        registry_manager_free(&registry_manager);
        return true;
    }

    RegistryLinkedList *list = registry_linked_list_create();
    VirtualRegistryArray *reg_arr;
    VirtualRegistryFilter *reg_filter;  
    
    if (list == NULL) {
        DP("ERROR: couldn't allocate memory for RegistryLinkedList @funcionalidade5()\n");
        registry_manager_free(&registry_manager);
        return false;
    }

    //Le todos os filtros de remocao dados pelo usuario e insere na linked list
    for (int i = 0; i < n; i++) {
        //Cria um filtro (false indica que o registro deve ser interpretado como filtro)
        reg_filter = virtual_registry_create_from_input(false); //false indica que os campos não informados devem ser ignorados
        if (reg_filter == NULL) {
            DP("ERROR: couldn't allocate memory for VirtualRegistry @funcionalidade5()\n");
            registry_manager_free(&registry_manager);
            registry_linked_list_delete(&list, true);
            return false;
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
        return false;
    }

    //Libera a memória da lista lidada sem apagar os seus itens pelo mesmo motivo acima
    registry_linked_list_delete(&list, false);

    //Remove os registros que contiverem as informações especificadas (remove os que derem match)
    registry_manager_remove_matches(registry_manager, reg_arr);

    //desaloca toda a memoria, fecha o arquivo e seta o status como consistente
    virtual_registry_array_delete(&reg_arr);
    registry_manager_free(&registry_manager);

    return true;
}

/*
    Funcionalidade 6: Inserir novo registro, ignorando espaços livres por deleção (abordagem de deleção estática)
    Parâmetros:
        char *bin_filename -> nome do arquivo binario
        char *n_str -> string contendo a quantidade (int) de registros a serem inseridos
        Funcionalidade10callbackInfo *extInfo -> struct de callback que pode ser informada para ser chamada a cada inserção (se NULL, é simplesmente ignorada).
    Retorno: bool -> indica se a funcionalidade foi executada com sucesso.
*/
static bool funcionalidade6 (char *bin_filename, char *n_str, Funcionalidade10callbackInfo *extInfo) {
    //Validação de parâmetros
    if (bin_filename == NULL) {
        DP("ERROR: invalid filename @funcionalidade6()\n");
        return false;
    }
    
    if (n_str == NULL) {
        DP("ERROR: invalid parameter @funcionalidade6()\n");
        return false;
    }

    //Tenta criar um RegistryManager
    RegistryManager *registry_manager = registry_manager_create();
    if (registry_manager == NULL) {
        DP("ERRO: unable to create RegistryManager @funcionalidade6\n");
        return false;
    }

    //Abre o arquivo para escrita, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, MODIFY);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        open_result_print_message(open_result);
        return false;
    }

    static char *campos[] = {"cidadeMae","cidadeBebe","idNascimento","idadeMae","dataNascimento","sexoBebe","estadoMae","estadoBebe"}; 

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
    return true;
}

/*
    Funcionalidade 7: Atualizar um registro pelo seu RRN
    Parâmetros:
        char *bin_filename -> nome do arquivo binario
        char *n_str -> string contendo a quantidade (int) de registros a serem atualizados
    Retorno: bool -> indica se a funcionalidade foi executada com sucesso.
*/
static bool funcionalidade7 (char *bin_filename, char *n_str) {
    //Validação de parâmetros
    if (bin_filename == NULL) {
        DP("ERROR: invalid filename @funcionalidade7()\n");
        return false;
    }
    
    if (n_str == NULL) {
        DP("ERROR: invalid parameter @funcionalidade7()\n");
        return false;
    }

    //Tenta criar o RegistryManager
    RegistryManager *registry_manager = registry_manager_create();
    if (registry_manager == NULL) {
        DP("ERRO: unable to create RegistryManager @funcionalidade7\n");
        return false;
    }


    //Abre o arquivo para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, bin_filename, MODIFY);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        open_result_print_message(open_result);
        return false;
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

    return true;
}

/**
 *  Funcionalidade 8: cria um novo índice de arvore-b e o alimenta
 *  com os registros presente no arquivo de registros.
 *  Parâmetros:
 *      char *reg_bin_filename -> nome do arquivo de registros já existente
 *      char *b_tree_filename -> nome do arquivo de indices a ser criado
 *  Retorno:
 *      bool -> indica se a funcionalidade foi executada com sucesso. 
 */
static bool funcionalidade8 (char *reg_bin_filename, char *b_tree_filename) {
    //Validação de parâmetros
    if (reg_bin_filename == NULL || b_tree_filename == NULL) {
        DP("ERROR: invalid filename @funcionalidade8()\n");
        return false;
    }

    //Cria um RegistryManager para ler todos os registros em disco
    RegistryManager *registry_manager = registry_manager_create();

    if (registry_manager == NULL) {
        DP("ERROR: couldn't create RegistryManager @funcionalidade8()\n");
        if (DEBUG) return false;
    }

    //Abre o arquivo de registros para leitura, caso a abertura não seja bem sucedida, exibe mensagem com o erro e interrompe o fluxo
    OPEN_RESULT open_result = registry_manager_open(registry_manager, reg_bin_filename, READ);
    if (open_result != OPEN_OK) {
        registry_manager_free(&registry_manager);
        open_result_print_message(open_result);
        return false;
    }

    //Cria um BTreeManager para gerenciar a btree em disco
    BTreeManager *b_tree_manager = b_tree_manager_create();

    if (b_tree_manager == NULL) {
        DP("ERROR: couldn't create BTreeManager @funcionalidade8()\n");
        return false;
    }

    //Tenta criar um novo arquivo de índices, se ocorrer algum erro, exibir como na especificação
    open_result = b_tree_manager_open(b_tree_manager, b_tree_filename, CREATE);
    if (open_result != OPEN_OK) {
        open_result_print_message(open_result);
        b_tree_manager_free(&b_tree_manager);
        return false;
    }

    //Obtém os headers do arquivo de registros que já está em RAM para fazer o loop
    RegistryHeader *reg_header = registry_manager_get_registry_header(registry_manager);
    int registryCount = reg_header_get_registries_count(reg_header);
    int RRN = -1;
    
    //Insere todos os registros do arquivo de registros no arquivo de índices
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

    return true;
}

/**
 *  Funcionalidade 9: busca um registro por seu RRN e o exibe na tela.
 *  a busca é feita em um arquivo de índices de registros (árvore-B).
 *  Após encontrar o registro no arquivo de índices, o RRN encontrado é
 *  usado no arquivo de registros para exibir demais informações daquele registro.
 *  Parâmetros:
 *      char *reg_filename -> nome do arquivo de registros
 *      char *b_tree_filename -> nome do arquivo de índices
 *      char *searchFieldStr -> nome do campo a ser buscado no arquivo de índices (atualmente suporta apenas idNascimento)
 *      char *searchVauleStr -> valor do campo a ser buscado no arquivo de índices 
 *  Retorno: bool -> indica se a funcionalidade foi executada com sucesso.
 */
static bool funcionalidade9 (char *reg_filename, char *b_tree_filename, char *searchFieldStr, char *searchValueStr) {
    //Validação de parâmetros
    if (reg_filename == NULL || b_tree_filename == NULL || searchFieldStr == NULL || searchValueStr == NULL) {
        DP("Invalid arguments @funcionalidade9()\n");
        return false;
    }

    //Checa se o usuário está procurando por um campo não suportado
    if (strcmp(searchFieldStr, "idNascimento") != 0) {
        DP("ERROR: trying to search in b-tree by unsuported field @funcionalidade9()\n");
        printf("Falha no processamento do arquivo\n");
        return false;
    }

    //Em caso de resultado NaN, a conversão vai dar 0
    int idNascimento = atoi(searchValueStr);
    if (searchValueStr[0] != '0' && idNascimento == 0) {
        DP("Trying to search a non-int idNascimento (idNascimentoStr = '%s')\n", searchValueStr);
        if (DEBUG) return false;
    }

    //Tenta cria um gerenciador de btree
    BTreeManager *btman = b_tree_manager_create();
    if (btman == NULL) {
        DP("ERROR: couldn't allocate memory for BTreeManager\n");
        return false;
    }

    //Tenta criar um gerenciador de registros
    RegistryManager *regman = registry_manager_create();
    if (regman == NULL) {
        DP("ERROR: couldn't allocate memory for RegistryManager\n");
        return false;
    }

    //Tenta abrir o arquivo de índices e o arquivo de registros, exibindo as mensagens de erro de acordo
    OPEN_RESULT o_res;
    o_res = b_tree_manager_open(btman, b_tree_filename, READ);
    if (o_res != OPEN_OK) {
        open_result_print_message(o_res);
        b_tree_manager_free(&btman);
        registry_manager_free(&regman);
        return false;
    }

    o_res = registry_manager_open(regman, reg_filename, READ);
    if (o_res != OPEN_OK) {
        open_result_print_message(o_res);
        b_tree_manager_free(&btman);
        registry_manager_free(&regman);
        return false;
    }

    //Obtém RRN desejado e o número de acessos a disco, respectivamente
    pairIntInt p = b_tree_manager_search_for(btman, idNascimento);

    //Se não for encontrado (RRN == -1)
    if (p.first == -1) {
        printf("Registro inexistente.\n");
        b_tree_manager_free(&btman);
        registry_manager_free(&regman);
        return false;
    }

    //Obtém o registro com o RRN encontrado
    VirtualRegistry *registry = registry_manager_fetch_at(regman, p.first);

    //Se não houve problemas em buscar o registro e ele não estiver deletado, printa na tela
    if (registry != NULL) {
        virtual_registry_print(registry);
    } 

    printf("Quantidade de paginas da arvore-B acessadas: %d\n", p.second);
    virtual_registry_free(&registry);    
    return true;
}

//Callback usado pela funcionalidade 10 para indicar para a funcionalidade6 o que deve ser feito após cada inserção
static void insertInBtreeCallback (Funcionalidade10callbackInfo *info) {
    b_tree_manager_insert(info->btman, info->idNascimento, info->RRN);
}

/**
 *  Funcionalidade 10: realiza a inserção um registro tanto no arquivo de registros
 *  quanto no arquivo de índices. Para tal, é chamada a funcionalidade6 com um extensor.
 *  O extensor indica o que deve ser feito após a inserção.
 *  Parâmetros:
 *    char *reg_filename -> nome do arquivo de registros.
 *    char *b_tree_filename -> nome do arquivo de índices.
 *  Retorno: bool -> indica se a funcionalidade foi executada com sucesso.
 * 
 */
static bool funcionalidade10(char *reg_filename, char *b_tree_filename, char *n_str) {
    //Tenta criar um gerenciador da btree
    BTreeManager *btman = b_tree_manager_create();
    if (btman == NULL) {
        DP("ERROR: couldn't allocate memory for BTreeManager\n");
        return false;
    }

    //Tenta abrir o arquivo de índices, se não conseguir, exibe mensagem correspondente
    OPEN_RESULT o_res;
    o_res = b_tree_manager_open(btman, b_tree_filename, MODIFY);
    if (o_res != OPEN_OK) {
        open_result_print_message(o_res);
        b_tree_manager_free(&btman);
        return false;
    }

    //Informações que são passadas para um callback da funcionalidade 6, que chama a funcionalidade10callback a cada inserção
    Funcionalidade10callbackInfo extensionInfo;
    extensionInfo.btman = btman;
    extensionInfo.callback = insertInBtreeCallback;
    //Valores inválidos (não são utilizados)
    extensionInfo.idNascimento = -1;
    extensionInfo.RRN = -1;

    //Chama a funcionalidade 6 (inserir no arquivo de registros), passando um callback (a cada inserção, o callback é chamado)
    bool success = funcionalidade6(reg_filename, n_str, &extensionInfo);
    b_tree_manager_free(&btman);
    return success; //O sucesso da função é determinado pela funcionalidade 6
}


/**
 *  Inicializa um vetor de parâmetros lidos do stdin
 *  Parâmetros:
 *      int quantity -> quantidade de parâmetros a serem lidos (tamanho do vetor)
 *  Retorno:
 *      char ** -> vetor com 'quantity' parâmetros
 */
char **prompt_params(int quantity){ 
    char **params = malloc(sizeof(char*) * quantity);

    if (params == NULL) {
        DP("ERROR: not enough memory @prompt_params!\n");
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
        //As funcionalidades que exigem binarioNaTela() retornam true se a função precisar ser chamada (se não houver erros)
        case 1: {
            params = prompt_params(2); //Lê dois parâmetros
            bool success = funcionalidade1(params[0], params[1]);
            if (success) binarioNaTela(params[1]);
            free_params(&params, 2);
            break;
        }
            
        case 2: {
            params = prompt_params(1); //Lê um parâmetro
            funcionalidade2(params[0]);
            free_params(&params, 1);
            break;
        }
            
        case 3: {
            params = prompt_params(1);
            funcionalidade3(params[0]);
            free_params(&params, 1);
            break;
        }

        case 4: {
            params = prompt_params(2);
            funcionalidade4(params[0], params[1]);
            free_params(&params, 2);
            break;
        }

        case 5: {
            params = prompt_params(2);
            bool success = funcionalidade5(params[0], params[1]);
            if (success) binarioNaTela(params[0]);
            free_params(&params, 2);
            break;
        }

        case 6: {
            params = prompt_params(2);
            bool success = funcionalidade6(params[0], params[1], NULL);
            if (success) binarioNaTela(params[0]);
            free_params(&params, 2);
            break;
        }
        
        case 7: {
            params = prompt_params(2);
            bool success = funcionalidade7(params[0], params[1]);
            if (success) binarioNaTela(params[0]);
            free_params(&params, 2);
            break;
        }

        case 8: {
            params = prompt_params(2);
            bool success = funcionalidade8(params[0], params[1]);
            if (success) binarioNaTela(params[1]);
            free_params(&params, 2);
            break;
        }

        case 9: {
            params = prompt_params(4);
            funcionalidade9(params[0], params[1], params[2], params[3]);
            free_params(&params, 4);
            break;
        }

        case 10: {
            params = prompt_params(3);
            bool success = funcionalidade10(params[0], params[1], params[2]);
            if (success) binarioNaTela(params[1]);
            free_params(&params, 3);
            break;
        }

        default:
            printf("Funcionalidade %c não implementada.\n", funcionalidade_code);
            break;
    }

    return 0;
}
