#include "csv_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "registry_linked_list.h"
#include "string_utils.h"
#include "debug.h"

/**
 *  Lê uma linha completa do CSV
 *  OBS: a linha deve ter, no máximo, 1024 caracteres. Demais caracteres serão ignorados
 *  Parâmetros: 
 *      FILE *file_stream -> stream de um arquivo com leitura ativada
 *  Retorno:
 *      VirtualRegistry* -> pointer para struct com informações lidas do registro
 */
VirtualRegistry *csv_read_line(FILE *file_stream) {
    
    //Buffer para leitura com fgets
    char buf[1025];

    //Se EOF, retorna NULL para enviar a mensagem para quem estiver usando esta função
    if (fgets(buf, 1024, file_stream) == NULL) return NULL;


    //Inicializa o registro com valores padrões
    VirtualRegistry *reg_data = virtual_registry_create();


    //OBS: strdups são necessários pois o token retornado aponta para uma região do buffer, que é estático (ou seja, vai ser liberado ao fim da função)
    reg_data->cidadeMae = strdup(_csv_registry_token(buf));
    reg_data->cidadeBebe = strdup(_csv_registry_token(NULL));
    

    //Variável para armazenamento temporário do token (necessária devido às checagens de string vazia abaixo)
    char *token = _csv_registry_token(NULL);

    //Se a idade não for informada ou for 0, mantenha o valor padrão (-1)
    if (!is_string_empty(token)) 
        reg_data->idNascimento = atoi(token);

    //Se a idade não for informada ou for 0, mantenha o valor padrão (-1)
    token = _csv_registry_token(NULL);
    if (!is_string_empty(token) && strcmp(token, "0") != 0)
        reg_data->idadeMae = atoi(token);
    if (reg_data->idadeMae == 0)
        reg_data->idadeMae = -1;

    reg_data->dataNascimento = strdup(_csv_registry_token(NULL));

    //Se sexo não for informado ou se for um valor inválido, mantenha o valor 0 (ignorado)
    token = _csv_registry_token(NULL);
    if (!is_string_empty(token) && strlen(token) == 1) {
        //Evitar valores indesejados (valores diferentes de 0, 1 e 2)
        if (token[0] == '1' || token[0] == '2')
            reg_data->sexoBebe = token[0];
    }

    reg_data->estadoMae = strdup(_csv_registry_token(NULL));
    reg_data->estadoBebe = strdup(_csv_registry_token(NULL));

    return reg_data;
}

/**
 *  Lê todas as linhas do csv, salvando cada uma em um VirtualRegistry. Em sequência, salva estes na struct CsvData e a retorna.
 *  Parâmetros:
 *      const char *file_name -> nome do arquivo csv a ser lido
 *  Retorno:
 *      CsvData* -> pointer para a struct que carrega os registros lidos
 */
VirtualRegistryArray *csv_read_all_lines(const char *file_name) {
    //1. Tenta abrir o arquivo csv
    if (file_name == NULL) {
        DP("ERROR: (parameter) invalid null file name @csv_read_all_lines!\n");
        return NULL;
    }
    
    FILE *csv_file = fopen(file_name, "r");
    //Nenhuma mensagem de erro é exibida pois o run.codes não aceitaria
    if (csv_file == NULL) return NULL;

    VirtualRegistry *dummy_headers;
    //Tenta ler os header, a fim de ignorá-los. Se não conseguir, o arquivo estava vazio (EOF encontrado).
    if ((dummy_headers = csv_read_line(csv_file)) == NULL) {
        fclose(csv_file);
        return virtual_registry_array_create(NULL, 0); //Retorna um vetor alocado, porém sem registros
    } else virtual_registry_delete(&dummy_headers);

    //2. Salva todos os registros em uma lista ligada
    RegistryLinkedList *list = registry_linked_list_create();
    
    //Lê o csv, linha a linha, salvando os dados em uma lista ligada, até encontrar EOF (igual a NULL).
    VirtualRegistry *curr_virtual_registry;
    while ((curr_virtual_registry = csv_read_line(csv_file)) != NULL)
        registry_linked_list_insert(list, curr_virtual_registry);

    fclose(csv_file);

    //3. Transforma a lista ligada em um vetor simples por simplicidade
    VirtualRegistryArray *registries = registry_linked_list_to_array(list);

    //Libera o espaço da lista ligada, que não será mais utilizada
    registry_linked_list_delete(&list, false);
    return registries;
}