#include "csv_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "registry_linked_list.h"
#include "string_utils.h"
#include "open_mode.h"

#include "debug.h"

typedef struct csv_reader_ CsvReader;

//Representa o TAD CsvReader
struct csv_reader_ {
    FILE *csv_file;
};


/**
 *  Factory de CsvReader, cria uma instância com valores padrão.
 *  Parâmetros: nenhum
 *  Retorno: 
 *      RegistryManager* -> instância criada 
 */
CsvReader *csv_reader_create(void) {
    CsvReader *reader = malloc(sizeof(CsvReader));
    reader -> csv_file = NULL;
    return reader;
}

/**
 *  Abre um arquivo csv, o qual será lido pelo CsvReader.
 *  Parâmetros:
 *      CsvReader *reader -> instância do leitor de csv
 *		char* csv_filename -> nome do arquivo (caminho completo)
 *  Retorno:
 *      OPEN_RESULT -> resultado da abertura (ler a documentação de OPEN_RESULT em open_mode.h)
 * 
 */
OPEN_RESULT csv_reader_open(CsvReader *reader, char *csv_filename) {
    if (csv_filename == NULL) {
        DP("ERROR: (parameter) invalid null file name @csv_read_all_lines!\n");
        return OPEN_INVALID_ARGUMENT;
    }
    
    reader->csv_file = fopen(csv_filename, "r");
    //Nenhuma mensagem de erro é exibida pois o run.codes não aceitaria
    if (reader->csv_file == NULL) {
        DP("ERROR: unable to open csv file @csv_reader_open()");
        return OPEN_FAILED;
    }

    //Ignora os headers (primeira linha)
    fscanf(reader->csv_file, "%*[^\n]\n");

    return OPEN_OK;
}

/**
 *  Lê uma linha completa do CSV
 *  OBS: a linha deve ter, no máximo, 1024 caracteres. Demais caracteres serão ignorados
 *  Parâmetros: 
 *      FILE *file_stream -> stream de um arquivo com leitura ativada
 *  Retorno:
 *      VirtualRegistry* -> pointer para struct com informações lidas do registro
 */
VirtualRegistry *csv_reader_readline(CsvReader *reader) {
    //Buffer para leitura com fgets
    static char buf[1025];

    //Se EOF, retorna NULL para enviar a mensagem para quem estiver usando esta função
    if (fgets(buf, 1024, reader->csv_file) == NULL) return NULL;

    //Inicializa o registro com valores padrões
    VirtualRegistry *registry = virtual_registry_create();

    //OBS: strdups são necessários pois o token retornado aponta para uma região do buffer, que é estático (ou seja, vai ser liberado ao fim da função)
    registry->cidadeMae = strdup(_csv_registry_token(buf));
    registry->cidadeBebe = strdup(_csv_registry_token(NULL));

    //Variável para armazenamento temporário do token (necessária devido às checagens de string vazia abaixo)
    char *token = _csv_registry_token(NULL);

    //Se a idade não for informada ou for 0, mantenha o valor padrão (-1)
    if (!is_string_empty(token)) 
        registry->idNascimento = atoi(token);

    //Se a idade não for informada ou for 0, mantenha o valor padrão (-1)
    token = _csv_registry_token(NULL);
    if (!is_string_empty(token) && strcmp(token, "0") != 0)
        registry->idadeMae = atoi(token);
    if (registry->idadeMae == 0)
        registry->idadeMae = -1;

    registry->dataNascimento = strdup(_csv_registry_token(NULL));

    //Se sexo não for informado ou se for um valor inválido, mantenha o valor 0 (ignorado)
    token = _csv_registry_token(NULL);
    if (!is_string_empty(token) && strlen(token) == 1) {
        //Evitar valores indesejados (valores diferentes de 0, 1 e 2)
        if (token[0] == '1' || token[0] == '2')
            registry->sexoBebe = token[0];
    }

    registry->estadoMae = strdup(_csv_registry_token(NULL));
    registry->estadoBebe = strdup(_csv_registry_token(NULL));

    return registry;
}

/**
 *  Fecha o arquivo csv.
 *  Parâmetros:
 *      CsvReader *reader -> gerenciador que possui o arquivo aberto
 *  Retorno: void
 */
void csv_reader_close(CsvReader *reader) {
    //Verifica se o reader já foi deletado ou se o arquivo já foi fechado
    if (reader == NULL || reader->csv_file == NULL) return;

    //fecha o arquivo
    fclose(reader->csv_file);

	//Marca qua não existe arquivo aberto
    reader->csv_file = NULL;
}

/*
	Funcao que desaloca a memoria de um leitor csv e fecha a stream se ainda estiver aberta. 
	Parametros:
		reader_ptr -> o endereco do leitor csv
	Retorno:
		nao ha retorno 
*/
void csv_reader_free(CsvReader **reader_ptr) {
	//Validação de parâmetros
	if (reader_ptr == NULL) {
		DP("ERROR: invalid parameter @csv_reader_free()\n");
		return;
	}
	#define reader (*reader_ptr)

	//Já foi liberado
	if (reader == NULL) return;

	csv_reader_close(reader);

	//Redefine os valores ao padrão inicial
	reader -> csv_file = NULL;
	free(reader);
	reader = NULL;
	#undef reader
}