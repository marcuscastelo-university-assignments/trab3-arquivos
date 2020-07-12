#include "binary_io.h"
#include <string.h>
#include <stdlib.h>

#include "registry_utils.h"
#include "string_utils.h"

#include "debug.h"

#define INF 1e9+5
#define GARBAGE_CHAR '$'


/**
 *  Escreve um int no disco, na posição atual do cursor
 *  Função auxiliar que muda o modo como o código é escrito,
 *  permitindo o uso de constantes como parâmetro. Torna o código mais legível
 *  que criar diversar variáveis ou reutilizar a mesma várias vezes.
 *  Parâmetros:
 *      FILE *file -> stream aberta em modo que permita escrita
 *      int num -> int a ser escrito
 *  Retorno: void
 */
void binary_write_int(FILE *file, int num) {
    if (file == NULL)
        return;

    fwrite(&num, sizeof(int), 1, file);

    return;
}

/**
 *  Lê um int do disco, na posição atual do cursor
 *  Função auxiliar que muda o modo como o código é escrito, 
 *  de modo a tornar o código mais limpo com atribuição de retorno de função
 *  em vez de alteração por referência.
 *  Parâmetros:
 *      FILE *file -> stream do arquivo binário aberta
 *  Retorno: int -> valor lido
 */
int binary_read_int(FILE *file) {
    if (file == NULL)
        return -INF;

    int num;
    fread(&num, sizeof(int), 1, file);

    return num;
}

/**
 *  Escreve um char no disco, na posição atual do cursor
 *  Função auxiliar que muda o modo como o código é escrito,
 *  permitindo o uso de constantes como parâmetro. Torna o código mais legível
 *  que criar diversar variáveis ou reutilizar a mesma várias vezes.
 *  Parâmetros:
 *      FILE *file -> stream aberta em modo que permita escrita
 *      char c -> char a ser escrito
 *  Retorno: void
 */
void binary_write_char(FILE *file, char c) {
    if (file == NULL)
        return;

    fwrite(&c, sizeof(char), 1, file);
}

/**
 *  Lê um char do disco, na posição atual do cursor
 *  Função auxiliar que muda o modo como o código é escrito, 
 *  de modo a tornar o código mais limpo com atribuição de retorno de função
 *  em vez de alteração por referência.
 *  Parâmetros:
 *      FILE *file -> stream do arquivo binário aberta
 *  Retorno: char -> valor lido
 */
char binary_read_char(FILE *file) {
    if (file == NULL)
        return '$';

    char c = '\0';
    fread(&c, sizeof(char), 1, file);

    return c;
}

/**
 *  Escreve uma string no disco, na posição atual do cursor
 *  Função auxiliar que muda o modo como o código é escrito,
 *  permitindo o uso de constantes como parâmetro. Torna o código mais legível
 *  que criar diversar variáveis ou reutilizar a mesma várias vezes.
 *  Parâmetros:
 *      FILE *file -> stream aberta em modo que permita escrita
 *      char *str -> string a ser escrita
 *      int size -> tamanho da string a ser lida (uma vez que o \0 não representa o fim da string a ser escrita) Ex.: estatoBebe = "\0$"
 *  Retorno: void
 */
void binary_write_string(FILE *file, char* str, int size) {
    if (file == NULL || str == NULL)
        return;

    fwrite(str, sizeof(char), size, file);

    return;
}

/**
 *  Lê uma string do disco, na posição atual do cursor
 *  Função auxiliar que muda o modo como o código é escrito, 
 *  de modo a tornar o código mais limpo com atribuição de retorno de função
 *  em vez de alteração por referência.
 *  Parâmetros:
 *      FILE *file -> stream do arquivo binário aberta
 *      int size -> tamanho da string a ser lida (uma vez que o \0 não é armazenado em disco, exceto em caso de string estática vazia)
 *  Retorno: string -> valor lido
 */
char *binary_read_string(FILE *file, int size) {
    if (file == NULL)
        return NULL;

    if (size == 0)
        return strdup("\0");

    char *str = (char*) malloc(sizeof(char) * size+1);
    
    if (str == NULL)
        return NULL;

    fread(str, sizeof(char), size, file);
    str[size] = '\0';

    return str;
}

/**
 *  Função auxiliar que calcula a quantidade de lixo ('$') que deve ser escrita dado o tamanho de
 *  cidadeMae + cidadeBebe, de modo a preencher todo o espaço para campos de tamanho variável.
 *  Parâmetros:
 *      int strings_size_sum -> a soma dos tamanhos das cidades
 *  Retorno:
 *      int -> tamanho do lixo a ser escrito
 */
int calculate_variable_fields_garbage(int strings_size_sum) {
    return REG_VARIABLE_FIELDS_TOTAL_SIZE-strings_size_sum-8;
}


/**
 *  Função auxiliar que gera uma string com lixo (padrão '$')
 *  dado o tamanho.
 *  
 *  Parâmetros:
 *      int garbage_size -> tamanho do lixo desejado
 *  Retorno:
 *      char* -> string alocada dinamicamente com garbage_size caracteres de lixo e um '\0' ao fim, 
 *      totalizando (garbage_size + 1) caracteres alocados. OBS: precisa ser liberada com free().
 * 
 */
char *generate_garbage(int garbage_size) {
    char *garbage_str = (char*) malloc(sizeof(char) * (1 + garbage_size));
    
    if (garbage_str == NULL)
        return NULL;
    
    for (int i = 0; i < garbage_size; i++) {   
        garbage_str[i] = GARBAGE_CHAR;    
    }
    
    garbage_str[garbage_size] = '\0';
    
    return garbage_str;
}