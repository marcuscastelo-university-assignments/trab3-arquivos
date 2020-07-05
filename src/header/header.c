#include "header.h"

#include <stdio.h>
#include <stdlib.h>
#include "binary_io.h"

#define HEADER_GARBAGE_SIZE 111

/**
 *  Struct encapsulada por um TAD que representa os headers do arquivo na RAM.
 *  deve ser encapsulada pois ao fazer o "set" dos valores dos headers, estes são marcados para escrita
 *  o que não seria possível se o acesso direto fosse permitido.
 *  Os campos abaixo são representações diretas dos headers presentes na especificação do trabalho, com
 *  excessão do membro changedMask, que é uma máscara de bits que indica quais headers foram modificados e, 
 *  portanto, precisam ser escritos.
 */
struct _header {
    ChangedHeadersMask changedMask;
    char status;
    int next_RRN;
    int registries_count;
    int removed_count;
    int updated_count;
};

/**
 *  Factory de criação do TAD descrito acima
 *  Parâmetros: void
 *  Retorno: Header* -> a instância criada pelo factory
 */
Header *header_create(void) {
    //Tenta alocar memória
    Header *header = malloc(sizeof(Header));
    if (header == NULL) {
        fprintf(stderr, "ERROR: not enough memory @header_create()\n");
        return NULL;
    }

    //Status -1 significa que o lixo ainda não foi escrito
    header->status = -1;
    header->next_RRN = 0;
    header->registries_count = 0;
    header->removed_count = 0;
    header->updated_count = 0;

    //Marca que, em um momento oportuno, todos os headers devem ser escritos 
    header->changedMask = HMASK_ALL;
    
    return header;
}


/**
 *  Libera a memória usada pelo TAD Header
 *  Parâmetros:
 *      Header **header_ptr -> referência à variável que guarda o pointer para o TAD
 *  Retorno: void
 */
void header_delete(Header **header_ptr) {
    //Validação de parâmetros
    if (header_ptr == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null pointer @header_delete()\n");
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
 *      Header *header -> struct que contêm os headers a serem escritos, se necessário.
 *      FILE *file -> stream com escrita permitida do arquivo binário destino
 *  Retorno: void
 */
void header_write_to_bin(Header *header, FILE *file) {
    //Validação de parâmetros
    if (header == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null header @header_write_to_bin()\n");
        return;
    }

    if (file == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null file stream @header_write_to_bin()\n");
        return;
    }

    //Verifica se é necessário escrever o lixo dos headers (ou seja, se o arquivo acabou de ser criado)
    bool shouldWriteGarbage = false;
    if (header->status == -1) { //O status -1 indica que o arquivo acabou de ser criado e, portanto, não possuia headers
        header_set_status(header, '0'); //Indica o arquivo como inconsistente
        shouldWriteGarbage = true;
    }

    //Indica os offsets usados para dar fseek quando necessário
    int offsets[6];
    offsets[0] = 0;                                 //Status '0' ou '1'
    offsets[1] = offsets[0] + 1 * sizeof(char);     //Próximo RRN
    offsets[2] = offsets[1] + 1 * sizeof(int);      //Contador de registros
    offsets[3] = offsets[2] + 1 * sizeof(int);      //Contador de registros removidos
    offsets[4] = offsets[3] + 1 * sizeof(int);      //Contador de registros atualizados
    offsets[5] = offsets[4] + 1 * sizeof(int);      //Lixo para completar 128 bytes
    
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
    if (header->changedMask & HMASK_STATUS) {
        if (shouldFseek) fseek(file, offsets[0], SEEK_SET); //Se for o primeiro a ser escrito ou o anterior foi pulado, faça fseek
        binary_write_char(file, header->status); //Escreve no disco
        shouldFseek = false; //O header não foi pulado, fseek não é mais necessário
    } else shouldFseek = true; //O header foi pulado, fseek se torna necessário

    //A lógica se repete...

    if (header->changedMask & HMASK_NEXTRRN) {
        if (shouldFseek) fseek(file, offsets[1], SEEK_SET);
        binary_write_int(file, header->next_RRN);
        shouldFseek = false;
    } else shouldFseek = true;

    if (header->changedMask & HMASK_REGISTRIESCOUNT) {
        if (shouldFseek) fseek(file, offsets[2], SEEK_SET);
        binary_write_int(file, header->registries_count);
        shouldFseek = false;
    } else shouldFseek = true;

    if (header->changedMask & HMASK_REMOVEDCOUNT) {
        if (shouldFseek) fseek(file, offsets[3], SEEK_SET);
        binary_write_int(file, header->removed_count);
        shouldFseek = false;
    } else shouldFseek = true;

    if (header->changedMask & HMASK_UPDATEDCOUNT) {
        if (shouldFseek) fseek(file, offsets[4], SEEK_SET);
        binary_write_int(file, header->updated_count);
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
    header->changedMask = HMASK_NONE;
}

/**
 *  Atualiza o valor dos headers, lendo diretamente do disco.
 *  Parâmetros:
 *      Header *header -> header para o qual será direcionada a informação no disco presente
 *      FILE *bin_file -> arquivo aberto do qual os headers serão lidos
 *  Retorno: void
 */
void header_read_from_bin(Header *header, FILE *bin_file) {
    //Validação de parâmetros
    if (header == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null header @header_read_from_bin()\n");
        return;
    }

    if (bin_file == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null file stream @header_read_from_bin()\n");
        return;
    }

    //Posiciona o cursor no inicio do arquivo para a leitura dos headers
    fseek(bin_file, 0, SEEK_SET);

    //Lê o valor de todos os headers a partir do disco, atualizando a struct
    header->status = binary_read_char(bin_file);
    header->next_RRN = binary_read_int(bin_file);
    header->registries_count = binary_read_int(bin_file);
    header->removed_count = binary_read_int(bin_file);
    header->updated_count = binary_read_int(bin_file);

    //Indica que nenhum header precisa ser escrito, pois todos foram atualizados
    header->changedMask = HMASK_NONE;
}

/*
	Simples função get, retorna o valor encapsulado (status)
    Parâmetros:
        Header *header -> pointer para a struct referida.
    Retorno:
        char -> status: '0' = inconsistente, '1' = consistente
*/
char header_get_status (Header *header) { return header->status; }

/*
	Simples função get, retorna o valor encapsulado (next_RRN)
    Parâmetros:
        Header *header -> pointer para a struct referida.
    Retorno:
        int -> o próximo RRN do arquivo
*/
int header_get_next_RRN (Header *header) { return header->next_RRN; }

/*
	Simples função get, retorna o valor encapsulado (registries_count)
    Parâmetros:
        Header *header -> pointer para a struct referida.
    Retorno:
        int -> a quantidade de registros
*/
int header_get_registries_count (Header *header) { return header->registries_count; }

/*
	Simples função get, retorna o valor encapsulado (removed_count)
    Parâmetros:
        Header *header -> pointer para a struct referida.
    Retorno:
        int -> a quantidade de registros removidos
*/
int header_get_removed_count (Header *header) { return header->removed_count; }

/*
	Simples função get, retorna o valor encapsulado (updated_count)
    Parâmetros:
        Header *header -> pointer para a struct referida.
    Retorno:
        int -> a quantidade de registros atualizados
*/
int header_get_updated_count (Header *header) { return header->updated_count; }

/*
	Simples função set, define o valor encapsulado (status).
    OBS: não escreve no disco, apenas altera seu valor na RAM e indica que o header deve ser escrito
    em um momento oportuno.
    Parâmetros:
        Header *header -> pointer para a struct referida.
        char new_status -> '0' = inconsistente, '1' = consistente
    Retorno: void
*/
void header_set_status(Header *header, char new_status) {
    //Validação de parâmetros
    if (header == NULL) {
        fprintf(stderr, "ERROR: (parameter) invalid null header @header_set_status()\n");
        return;
    }

    if (new_status != '0' && new_status != '1') {
        fprintf(stderr, "ERROR: (parameter) invalid status provided, should be '0' or '1' @header_set_status()\n");
        return;
    }

    header->status = new_status;

    //Marca que o header precisará ser escrito em um momento oportuno.
    header->changedMask |= HMASK_STATUS;
}

//Função interna usada para interpretar os macros H_INCREASE e H_DECREASE, mantendo a função de atualizar o valor diretamente
int _parse_counter(int current_value, int counter) {
    //Como os macros H_INCREASE e H_DECREASE são negativos, os valores positivos e 0 ainda podem ser diretamente atribuídos
    if (counter >= 0) return counter;
	else if (counter == H_INCREASE) return current_value + 1;
    else if (counter == H_DECREASE) return current_value - 1;
	else return -1;
}

/*
	Simples função set, define o valor encapsulado (next_RRN).
    OBS: não escreve no disco, apenas altera seu valor na RAM e indica que o header deve ser escrito
    em um momento oportuno.
    Parâmetros:
        Header *header -> pointer para a struct referida.
        int new_value -> novo valor para o header, se >= 0. Se for H_INCREASE ou H_DECREASE fará o incremento ou o decremento, respectivamente.
    Retorno: void
*/
void header_set_next_RRN (Header *header, int new_value) {
	header->next_RRN = _parse_counter(header->next_RRN, new_value);

    //Marca que o header precisará ser escrito em um momento oportuno.
    header->changedMask |= HMASK_NEXTRRN;
}

/*
	Simples função set, define o valor encapsulado (registries_count).
    OBS: não escreve no disco, apenas altera seu valor na RAM e indica que o header deve ser escrito
    em um momento oportuno.
    Parâmetros:
        Header *header -> pointer para a struct referida.
        int new_value -> novo valor para o header, se >= 0. Se for H_INCREASE ou H_DECREASE fará o incremento ou o decremento, respectivamente.
    Retorno: void
*/
void header_set_registries_count (Header *header, int new_value) {
	header->registries_count = _parse_counter(header->registries_count, new_value);

    //Marca que o header precisará ser escrito em um momento oportuno.
    header->changedMask |= HMASK_REGISTRIESCOUNT;
}

/*
	Simples função set, define o valor encapsulado (removed_count).
    OBS: não escreve no disco, apenas altera seu valor na RAM e indica que o header deve ser escrito
    em um momento oportuno.
    Parâmetros:
        Header *header -> pointer para a struct referida.
        int new_value -> novo valor para o header, se >= 0. Se for H_INCREASE ou H_DECREASE fará o incremento ou o decremento, respectivamente.
    Retorno: void
*/
void header_set_removed_count (Header *header, int new_value) {
	header->removed_count = _parse_counter(header->removed_count, new_value);

    //Marca que o header precisará ser escrito em um momento oportuno.
    header->changedMask |= HMASK_REMOVEDCOUNT;
}

/*
	Simples função set, define o valor encapsulado (updated_count).
    OBS: não escreve no disco, apenas altera seu valor na RAM e indica que o header deve ser escrito
    em um momento oportuno.
    Parâmetros:
        Header *header -> pointer para a struct referida.
        int new_value -> novo valor para o header, se >= 0. Se for H_INCREASE ou H_DECREASE fará o incremento ou o decremento, respectivamente.
    Retorno: void
*/
void header_set_updated_count (Header *header, int new_value) {
	header->updated_count = _parse_counter(header->updated_count, new_value);

    //Marca que o header precisará ser escrito em um momento oportuno.
    header->changedMask |= HMASK_UPDATEDCOUNT;
}
