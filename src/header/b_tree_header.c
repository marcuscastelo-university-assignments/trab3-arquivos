#include "b_tree_header.h"

#include <stdio.h>
#include <stdlib.h>
#include "binary_io.h"
#include "debug.h"

#define HEADER_GARBAGE_SIZE 55

/**
 *  Struct encapsulada por um TAD que representa os headers do arquivo na RAM.
 *  deve ser encapsulada pois ao fazer o "set" dos valores dos headers, estes são marcados para escrita
 *  o que não seria possível se o acesso direto fosse permitido.
 *  Os campos abaixo são representações diretas dos headers presentes na especificação do trabalho, com
 *  excessão do membro changedMask, que é uma máscara de bits que indica quais headers foram modificados e, 
 *  portanto, precisam ser escritos.
 */
struct _b_tree_header {
    ChangedBTHeadersMask changedMask;
    char status;
    int noRaiz;
    int nroNiveis;
    int proxRRN;
    int nroChaves;
};

/**
 *  Factory de criação do TAD descrito acima
 *  Parâmetros: void
 *  Retorno: BTreeHeader* -> a instância criada pelo factory
 */
BTreeHeader *b_tree_header_create(void) {
    //Tenta alocar memória
    BTreeHeader *header = malloc(sizeof(BTreeHeader));
    if (header == NULL) {
        DP("ERROR: not enough memory @header_create()\n");
        return NULL;
    }

    //Status -1 significa que o lixo ainda não foi escrito (valor é tratado apenas em RAM)
    header->status = -1;
    header->noRaiz = -1;
    header->nroNiveis = 0;
    header->proxRRN = 0;
    header->nroChaves = 0;

    //Marca que, em um momento oportuno, todos os headers devem ser escritos (supondo que é um arquivo novo, por enquanto)
    header->changedMask = BTHMASK_ALL;

    return header;
}


/**
 *  Libera a memória usada pelo TAD BTreeHeader
 *  Parâmetros:
 *      BTreeHeader **header_ptr -> referência à variável que guarda o pointer para o TAD
 *  Retorno: void
 */
void b_tree_header_free(BTreeHeader **header_ptr) {
    //Validação de parâmetros
    if (header_ptr == NULL) {
        DP("ERROR: (parameter) invalid null pointer @b_tree_header_free()\n");
        return;
    }

    //Libera a memória
    #define header (*header_ptr)
    free(header);
    header = NULL;
    #undef header
}

/**
 *  Função otimizada para evitar escritas desnecessárias ao disco
 *  Para isso, cada campo tem um indicador de modificação. Se o campo tiver sido modificado, ele precisa ser escrito.
 *  Parâmetros:
 *      BTreeHeader *header -> struct que contêm os headers a serem escritos, se necessário.
 *      FILE *file -> stream com escrita permitida do arquivo binário destino
 *  Retorno: void
 */
void b_tree_header_write_to_bin(BTreeHeader *header, FILE *file) {
    //Validação de parâmetros
    if (header == NULL) {
        DP("ERROR: (parameter) invalid null header @b_tree_header_write_to_bin()\n");
        return;
    }

    if (file == NULL) {
        DP("ERROR: (parameter) invalid null file stream @b_tree_header_write_to_bin()\n");
        return;
    }

    //Verifica se é necessário escrever o lixo dos headers (ou seja, se o arquivo acabou de ser criado)
    bool shouldWriteGarbage = false;
    if (header->status == -1) { //O status -1 indica que o arquivo acabou de ser criado e, portanto, não possuia headers
        b_tree_header_set_status(header, '0'); //Indica o arquivo como inconsistente
        shouldWriteGarbage = true;
    }

    //Indica os offsets usados para dar fseek quando necessário
    int offsets[6];
    offsets[0] = 0;                                 //Status '0' ou '1'
    offsets[1] = offsets[0] + 1 * sizeof(char);     //noRaiz
    offsets[2] = offsets[1] + 1 * sizeof(int);      //nroNiveis
    offsets[3] = offsets[2] + 1 * sizeof(int);      //proxRRN
    offsets[4] = offsets[3] + 1 * sizeof(int);      //nroChaves
    offsets[5] = offsets[4] + 1 * sizeof(int);      //Lixo

    //Código otimizado para o uso mínimo de fseeks, usando máscara de bits para decidir quais headers precisam ser atualizados
    /*
        Resumo da lógica: sempre dá fseek no primeiro header que precisar de alteração, 
        pois o cursor ainda não foi alterado. Dessa forma, se nenhum header precisar ser escrito, nenhum fseek é feito.
        Ao escrever um header que não o primeiro a ser escrito, verifica se o header que vem antes de si mesmo precisou ser
        escrito. Em caso positivo, um fseek não é necessário, visto que o cursor já está na posição certa. Porém em caso negativo,
        no qual um header foi "pulado" anteriormente, o fseek é necessário.
    */

    //Certifica que o primeiro header que precisar ser escrito fará fseek
    bool shouldFseek = true;

    //Se o campo status foi marcado para escrita
    if (header->changedMask & BTHMASK_STATUS) {
        if (shouldFseek) fseek(file, offsets[0], SEEK_SET); //Se for o primeiro a ser escrito ou o anterior foi pulado, faça fseek
        binary_write_char(file, header->status); //Escreve no disco
        shouldFseek = false; //O header não foi pulado, fseek não é mais necessário
    } else shouldFseek = true; //O header foi pulado, fseek se torna necessário

    //A lógica se repete...

    if (header->changedMask & BTHMASK_NORAIZ) {
        if (shouldFseek) fseek(file, offsets[1], SEEK_SET);
        binary_write_int(file, header->noRaiz);
        shouldFseek = false;
    } else shouldFseek = true;

    if (header->changedMask & BTHMASK_NRONIVEIS) {
        if (shouldFseek) fseek(file, offsets[2], SEEK_SET);
        binary_write_int(file, header->nroNiveis);
        shouldFseek = false;
    } else shouldFseek = true;

    if (header->changedMask & BTHMASK_PROXRRN) {
        if (shouldFseek) fseek(file, offsets[3], SEEK_SET);
        binary_write_int(file, header->proxRRN);
        shouldFseek = false;
    } else shouldFseek = true;

    if (header->changedMask & BTHMASK_NROCHAVES) {
        if (shouldFseek) fseek(file, offsets[4], SEEK_SET);
        binary_write_int(file, header->nroChaves);
        shouldFseek = false;
    } else shouldFseek = true;

    //Se for necessário escreve o lixo após os headers
    if (shouldWriteGarbage) {
        if (shouldFseek) fseek(file, offsets[5], SEEK_SET);
        char *garbage = generate_garbage(HEADER_GARBAGE_SIZE);
        binary_write_string(file, garbage, HEADER_GARBAGE_SIZE);
        free(garbage);
    }

    //Visto que os dados foram escritos no disco, eles se tornam atualizados
    header->changedMask = BTHMASK_NONE;
}

/**
 *  Atualiza o valor dos headers, lendo diretamente do disco.
 *  Parâmetros:
 *      BTreeHeader *header -> header para o qual será direcionada a informação no disco presente
 *      FILE *bin_file -> arquivo aberto do qual os headers serão lidos
 *  Retorno: void
 */
void b_tree_header_read_from_bin(BTreeHeader *header, FILE *bin_file) {
    //Validação de parâmetros
    if (header == NULL) {
        DP("ERROR: (parameter) invalid null header @b_tree_header_read_from_bin()\n");
        return;
    }

    if (bin_file == NULL) {
        DP("ERROR: (parameter) invalid null file stream @b_tree_header_read_from_bin()\n");
        return;
    }

    //Posiciona o cursor no inicio do arquivo para a leitura dos headers
    fseek(bin_file, 0, SEEK_SET);

    //Lê o valor de todos os headers a partir do disco, atualizando a struct
    header->status = binary_read_char(bin_file);
    header->noRaiz = binary_read_int(bin_file);
    header->nroNiveis = binary_read_int(bin_file);
    header->proxRRN = binary_read_int(bin_file);
    header->nroChaves = binary_read_int(bin_file);

    //Indica que nenhum header precisa ser escrito, pois todos foram atualizados
    header->changedMask = BTHMASK_NONE;
}

/*
	Simples função get, retorna o valor encapsulado (status)
    Parâmetros:
        BTreeHeader *header -> pointer para a struct referida.
    Retorno:
        char -> status: '0' = inconsistente, '1' = consistente
*/
char b_tree_header_get_status (BTreeHeader *header) { return header->status; }

/*
	Simples função get, retorna o valor encapsulado (noRaiz)
    Parâmetros:
        BTreeHeader *header -> pointer para a struct referida.
    Retorno:
        int -> o RRN do nó raiz
*/
int b_tree_header_get_noRaiz (BTreeHeader *header) { return header->noRaiz; }

/*
	Simples função get, retorna o valor encapsulado (nroNiveis)
    Parâmetros:
        BTreeHeader *header -> pointer para a struct referida.
    Retorno:
        int -> a quantidade de níveis da btree
*/
int b_tree_header_get_nroNiveis (BTreeHeader *header) { return header->nroNiveis; }

/*
	Simples função get, retorna o valor encapsulado (proxRRN)
    Parâmetros:
        BTreeHeader *header -> pointer para a struct referida.
    Retorno:
        int -> o próximo RRN a ser criado
*/
int b_tree_header_get_proxRRN (BTreeHeader *header) { return header->proxRRN; }

/*
	Simples função get, retorna o valor encapsulado (nroChaves)
    Parâmetros:
        BTreeHeader *header -> pointer para a struct referida.
    Retorno:
        int -> a quantidade de chaves
*/
int b_tree_header_get_nroChaves (BTreeHeader *header) { return header->nroChaves; }

/*
	Simples função set, define o valor encapsulado (status).
    OBS: não escreve no disco, apenas altera seu valor na RAM e indica que o header deve ser escrito
    em um momento oportuno.
    Parâmetros:
        BTreeHeader *header -> pointer para a struct referida.
        char new_status -> '0' = inconsistente, '1' = consistente
    Retorno: void
*/
void b_tree_header_set_status(BTreeHeader *header, char new_status) {
    //Validação de parâmetros
    if (header == NULL) {
        DP("ERROR: (parameter) invalid null header @b_tree_header_set_status()\n");
        return;
    }

    if (new_status != '0' && new_status != '1') {
        DP("ERROR: (parameter) invalid status provided, should be '0' or '1' @b_tree_header_set_status()\n");
        return;
    }

    header->status = new_status;

    //Marca que o header precisará ser escrito em um momento oportuno.
    header->changedMask |= BTHMASK_STATUS;
}

//Função interna usada para interpretar os macros H_INCREASE e H_DECREASE, mantendo a função de atualizar o valor diretamente
static int _parse_counter(int current_value, int counter) {
    //Como os macros H_INCREASE e H_DECREASE são negativos, os valores positivos e 0 ainda podem ser diretamente atribuídos
    if (counter >= 0) return counter;
	else if (counter == H_INCREASE) return current_value + 1;
    else if (counter == H_DECREASE) return current_value - 1;
	else return -1;
}

/*
	Simples função set, define o valor encapsulado (noRaiz).
    OBS: não escreve no disco, apenas altera seu valor na RAM e indica que o header deve ser escrito
    em um momento oportuno.
    Parâmetros:
        BTreeHeader *header -> pointer para a struct referida.
        int new_value -> novo valor para o header, se >= 0. Se for H_INCREASE ou H_DECREASE fará o incremento ou o decremento, respectivamente.
    Retorno: void
*/
void b_tree_header_set_noRaiz (BTreeHeader *header, int new_value) {
	header->noRaiz = _parse_counter(header->noRaiz, new_value);

    //Marca que o header precisará ser escrito em um momento oportuno.
    header->changedMask |= BTHMASK_NORAIZ;
}

/*
	Simples função set, define o valor encapsulado (nroNiveis).
    OBS: não escreve no disco, apenas altera seu valor na RAM e indica que o header deve ser escrito
    em um momento oportuno.
    Parâmetros:
        BTreeHeader *header -> pointer para a struct referida.
        int new_value -> novo valor para o header, se >= 0. Se for H_INCREASE ou H_DECREASE fará o incremento ou o decremento, respectivamente.
    Retorno: void
*/
void b_tree_header_set_nroNiveis (BTreeHeader *header, int new_value) {
	header->nroNiveis = _parse_counter(header->nroNiveis, new_value);

    //Marca que o header precisará ser escrito em um momento oportuno.
    header->changedMask |= BTHMASK_NRONIVEIS;
}

/*
	Simples função set, define o valor encapsulado (proxRRN).
    OBS: não escreve no disco, apenas altera seu valor na RAM e indica que o header deve ser escrito
    em um momento oportuno.
    Parâmetros:
        BTreeHeader *header -> pointer para a struct referida.
        int new_value -> novo valor para o header, se >= 0. Se for H_INCREASE ou H_DECREASE fará o incremento ou o decremento, respectivamente.
    Retorno: void
*/
void b_tree_header_set_proxRRN (BTreeHeader *header, int new_value) {
	header->proxRRN = _parse_counter(header->proxRRN, new_value);

    //Marca que o header precisará ser escrito em um momento oportuno.
    header->changedMask |= BTHMASK_PROXRRN;
}

/*
	Simples função set, define o valor encapsulado (nroChaves).
    OBS: não escreve no disco, apenas altera seu valor na RAM e indica que o header deve ser escrito
    em um momento oportuno.
    Parâmetros:
        BTreeHeader *header -> pointer para a struct referida.
        int new_value -> novo valor para o header, se >= 0. Se for H_INCREASE ou H_DECREASE fará o incremento ou o decremento, respectivamente.
    Retorno: void
*/
void b_tree_header_set_nroChaves (BTreeHeader *header, int new_value) {
	header->nroChaves = _parse_counter(header->nroChaves, new_value);

    //Marca que o header precisará ser escrito em um momento oportuno.
    header->changedMask |= BTHMASK_NROCHAVES;
}