#include "string_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <bool.h>

void binarioNaTela(const char *nomeArquivoBinario) {

	/* Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo anteriormente.
	*  Ela vai abrir de novo para leitura e depois fechar (você não vai perder pontos por isso se usar ela). */

	unsigned long i, cs;
	unsigned char *mb;
	size_t fl;
	FILE *fs;
	if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
		return;
	}
	fseek(fs, 0, SEEK_END);
	fl = ftell(fs);
	fseek(fs, 0, SEEK_SET);
	mb = (unsigned char *) malloc(fl);
	fread(mb, 1, fl, fs);

	cs = 0;
	for(i = 0; i < fl; i++) {
		cs += (unsigned long) mb[i];
	}
	printf("%lf\n", (cs / (double) 100));
	free(mb);
	fclose(fs);
}

/**
 *	Função similar ao strtok voltada para o csv_reader.
 *	Obtém tokens de uma string, usando os delimitadores '\0','\n','\r' e ','.
 * 	Diferentemente do strtok, no caso de a string ser ",,", existem 4 tokens: "\0", "\0", e "\n" e "\0"
 * 	OBS: não são realizadas cópias durante a identificação dos tokens, portanto a memória utilizada por eles é a mesma da string interna
 * 	OBS: a string interna é modificada durante o procedimento, trocando os delimitadores por '\0'
 * 	Parâmetros:
 * 		char *string -> se NULL, mantenha a string interna, se diferente de NULL, atualize a string interna
 * 	Retorno:
 * 		char * -> posição na string interna do próximo token
 */
char *_csv_registry_token(char *string) {
    static char *_internal_string = NULL;
    if (string != NULL) _internal_string = string;
	//
    char *token_start = _internal_string;
    int token_size = 0;

	//Define o tamanho do token e posiciona a string no delimitador à direita
    while (*_internal_string != '\0' && *_internal_string != '\n' && *_internal_string != '\r' && *_internal_string != ',') {
		_internal_string++;
        token_size++;
	}
    
    //Escreve um caracter nulo para definir o fim da substring e prepara a posição para o próximo token
	*_internal_string++ = '\0';

    //Retorna a posição do começo do token
    return token_start;
}

/**
 * 	Função simples que verifica se uma string é composta apenas de um '\0'
 * 	Parâmetros:
 * 		char *string -> string a se verificar
 * 	Retorno:
 * 		char -> representação booleana (0: falso ou 1: true), indicando se a string é vazia
 */
int is_string_empty(char *string) {
    return string == NULL || !strcmp(string, "");
}

/*
	Essa funcao trata o valor sexoBebe, retornando uma string de acordo com seu valor
	Parametros:
		sexoBebe -> o char que representa o sexo do bebe
	Retorno:
		char* -> retorna "IGNORADO", "MASCULINO", "FEMININO", "INVALIDO" para o sexo de acordo com o valor passado no parametro
*/
char *parse_sexoBebe_for_print (char sexoBebe) {
	if (sexoBebe == '0') return "IGNORADO";
	if (sexoBebe == '1') return "MASCULINO";
	if (sexoBebe == '2') return "FEMININO";
	return "INVALIDO";
}


/*
	Funcao que verifica o valor de uma string de um registro, evitando a tentativa de exibicao de
	valores nulos, e que seja exibido de acordo com as especificacoes do trabalho
	Parametros:
		value -> a string do valor a ser tratado
	Retorno:
		char* -> a string apos o tratamento do valor
*/
char *parse_string_for_print(char *value) {
	if (value == NULL) return "NULO";
	if (is_string_empty(value) == true) return "-";
	return value;
}

void scan_quote_string(char **str_ptr) {
	#define str (*str_ptr)

	/*
	*	Use essa função para ler um campo string delimitado entre aspas (").
	*	Chame ela na hora que for ler tal campo. Por exemplo:
	*
	*	A entrada está da seguinte forma:
	*		nomeDoCampo "MARIA DA SILVA"
	*
	*	Para ler isso para as strings já alocadas str1 e str2 do seu programa, você faz:
	*		scanf("%s", str1); // Vai salvar nomeDoCampo em str1
	*		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2 (sem as aspas)
	*
	*/

	char R;

	while((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

	if(R == 'N' || R == 'n') { // campo NULO
		getchar(); getchar(); getchar(); // ignorar o "ULO" de NULO.
		str = strdup(""); // copia string vazia
	} else if(R == '\"') {
		if(scanf("%m[^\"]", &str) != 1) { // ler até o fechamento das aspas
			free(str);
			str = strdup("");
		}
		getchar(); // ignorar aspas fechando
	} else if(R != EOF){ // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então, pois deve ser algum inteiro ou algo assim...
		char *RS = strdup((char[]){R,0});
		scanf("%ms", &str);
		strcat(RS, str);
		free(str);
		str = RS;
	} else { // EOF
		str = strdup("");
	}
	#undef str
}

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
void open_result_print_message(OPEN_RESULT open_result) {
    if (open_result & (OPEN_FAILED | OPEN_INCONSISTENT)) printf("Falha no processamento do arquivo.\n");
    else if (open_result & OPEN_INVALID_ARGUMENT) DP("ERRO: Um argumento inválido foi informado\n");
}
