#include "binary_io.h"
#include <string.h>
#include <stdlib.h>

#include "registry.h"
#include "registry_utils.h"
#include "string_utils.h"

#define INF 1e9+5
#define VARIABLE_FIELDS_TOTAL_SIZE 105
#define GARBAGE_CHAR '$'

/**
 *  Função de baixo nível extremamente otimizada para o acesso mínimo a disco.
 *  Inicialmente, é feita a leitura do registro presente no disco. Em seguida,
 *  Analisa-se quais campos estão marcados para atualização no novo registro.
 *  Depois, é feita a atualização dos referidos campos, realizando o mínimo de fseeks
 *  e fwrites possível. 
 *  OBS: o cursor deve estar posicionado corretamente antes de chamar esta função
 *  OBS: se um campo variável antes era "12345" e é atualizado para "ABC", no disco ele manterá o final anterior,
 *  ficando "ABC45". Isso não faz diferença, uma vez que serão lidos apenas 3 caracteres do novo valor em leituras futuras.
 *  Parâmetros:
 *      FILE *file -> stream do arquivo binário aberta com modo que permita escrita 
 *      VirtualRegistryUpdater *updated_reg -> registro (novos dados) com máscara de bits indicando quais campos devem ser atualizados    
 *  Retorno: void
 */
void binary_update_registry(FILE *file, VirtualRegistryUpdater *updated_reg) {
    if (file == NULL || updated_reg == NULL)
        return;

    //Se não houver nenhum campo a ser atualizado, não faça nenhum acesso a disco
    if (updated_reg->fieldMask == MASK_NONE) return;

    //Armazena posição absoluta do começo do registro
    int registry_seek_start = ftell(file);

    //Lê o registro antigo para ver quais campos precisam ser modificados (uma vez que a leitura é mais barata que a escrita, é melhor não escrever um valor se o que estiver no disco for idêntico)
    VirtualRegistry *old_reg = binary_read_registry(file);

    //Se o registro estiver marcado como removido, não faça qualquer atualização.
    if (old_reg == NULL) return;

    //Obtém o tamanho dos campos variáveis antes e depois da atualização.
    int cidadeMaeOldSize = strlen(old_reg->cidadeMae);
    int cidadeBebeOldSize = strlen(old_reg->cidadeBebe);

    int cidadeMaeNewSize = strlen(updated_reg->cidadeMae);
    int cidadeBebeNewSize = strlen(updated_reg->cidadeBebe);

    //Define-se a posição de cada campo relativa ao início do registro (para futuros fseeks)
    int offsets[10];
    offsets[0] =              0 * sizeof(int);                                         //cidadeMaeSize
    offsets[1] = offsets[0] + 1 * sizeof(int);                                         //cidadeBebeSize
    offsets[2] = offsets[1] + 1 * sizeof(int);                                         //cidadeMae
    offsets[3] = offsets[2] + cidadeMaeNewSize * sizeof(char);                         //cidadeBebe
    offsets[4] = VARIABLE_FIELDS_TOTAL_SIZE;                                           //idNasc
    offsets[5] = offsets[4] + 1 * sizeof(int);                                         //idadeMae          
    offsets[6] = offsets[5] + 1 * sizeof(int);                                         //dataNasc  
    offsets[7] = offsets[6] + 10 * sizeof(char);                                       //sexoBebe      
    offsets[8] = offsets[7] + 1 * sizeof(char);                                        //estadoMae  
    offsets[9] = offsets[8] + 2 * sizeof(char);                                        //estadoBebe

    //Variáveis de controle que determinam a necessidade de re-escrita dos campos variáveis 
    bool shouldRewriteCidadeMae = false, shouldRewriteCidadeBebe = false;
    char *writeCidadeMae = NULL, *writeCidadeBebe = NULL;

    //Se cidadeMãe foi alterada, deve ser reescrita
    if ((updated_reg->fieldMask & MASK_CIDADEMAE) && strcmp(old_reg->cidadeMae, updated_reg->cidadeMae) != 0) {
        shouldRewriteCidadeMae = true;
        writeCidadeMae = updated_reg->cidadeMae;

        //Se o tamanho de cidadeMãe foi alterada, a cidadeBebe precisa ser reescrita (mesmo que inalterada)
        if (cidadeMaeNewSize != cidadeMaeOldSize) {
            shouldRewriteCidadeBebe = true;
            writeCidadeBebe = old_reg->cidadeBebe;
        } 
    }

    //Se cidadeBebe foi alterada, precisa ser reescrita
    if ((updated_reg->fieldMask & MASK_CIDADEBEBE) && strcmp(old_reg->cidadeBebe, updated_reg->cidadeBebe) != 0) {
        shouldRewriteCidadeBebe = true;
        writeCidadeBebe = updated_reg->cidadeBebe;
    }

    /*
        Verifica se a atualização ocupou todo o espaço dos campos variáveis ou se sobrou lixo anterior.
        Em caso de ter ocupado, fseek não é necessário para o primeiro campo estático.
        Se nãp tiver sido totalmente ocupado, é necessário realizar um fseek para o primeiro registro estático, 
        uma vez que o cursor ainda não o atingiu.
    */
    bool shouldFseekForFirstStatic = cidadeBebeNewSize + cidadeMaeNewSize == VARIABLE_FIELDS_TOTAL_SIZE;

    //Código otimizado para o uso mínimo de fseeks, usando máscara de bits para decidir quais campos precisam ser atualizados
    /*
        Resumo da lógica: sempre dá fseek no primeiro campo que precisar de alteração, 
        pois o cursor ainda não foi alterado. Dessa forma, se nenhum header precisar ser escrito, nenhum fseek é feito.
        Ao escrever um campo que não o primeiro a ser escrito, verifica se o campo que vem antes de si mesmo precisou ser
        escrito. Em caso positivo, um fseek não é necessário, visto que o cursor já está na posição certa. Porém em caso negativo,
        no qual um campo foi "pulado" anteriormente, o fseek é necessário.
    */

    //Certifica que o primeiro campo que precisar ser escrito fará fseek
    bool shouldFseek = true;

    //Se o campo cidadebebe foi marcado para escrita (e nesse caso, somente se o tamanho mudou)
    if ((updated_reg->fieldMask & MASK_CIDADEMAE) && cidadeMaeNewSize != cidadeMaeOldSize) {
        if (shouldFseek) fseek(file, registry_seek_start + offsets[0], SEEK_SET); //Se for o primeiro a ser escrito ou o anterior foi pulado, faça fseek
        binary_write_int(file, cidadeMaeNewSize); //Escreve no disco
        shouldFseek = false; //O campo não foi pulado, fseek não é mais necessário
    }
    else shouldFseek = true; //O campo foi pulado, fseek se torna necessário

    //A lógica se repete...

    if ((updated_reg->fieldMask & MASK_CIDADEBEBE) && cidadeBebeNewSize != cidadeBebeOldSize) {
        if (shouldFseek) fseek(file, registry_seek_start + offsets[1], SEEK_SET);
        binary_write_int(file, cidadeBebeNewSize);
        shouldFseek = false;
    }
    else shouldFseek = true;

    if (shouldRewriteCidadeMae) {
        if (shouldFseek) fseek(file, registry_seek_start + offsets[2], SEEK_SET);
        binary_write_string(file, writeCidadeMae, strlen(writeCidadeMae));
        shouldFseek = false;
    }
    else shouldFseek = true;

    if (shouldRewriteCidadeBebe) {
        if (shouldFseek) fseek(file, registry_seek_start + offsets[3], SEEK_SET);
        binary_write_string(file, writeCidadeBebe, strlen(writeCidadeBebe));
        shouldFseek = false;
    }
    else shouldFseek = true;

    if ((updated_reg->fieldMask & MASK_IDNASCIMENTO) && old_reg->idNascimento != updated_reg->idNascimento) {
        if (shouldFseekForFirstStatic) fseek(file, registry_seek_start + offsets[4], SEEK_SET);
        binary_write_int(file, updated_reg->idNascimento);
        shouldFseek = false;
    }
    else shouldFseek = true;

    if ((updated_reg->fieldMask & MASK_IDADEMAE) && old_reg->idadeMae != updated_reg->idadeMae) {
        if (shouldFseek) fseek(file, registry_seek_start + offsets[5], SEEK_SET);
        binary_write_int(file, updated_reg->idadeMae);
        shouldFseek = false;
    }
    else shouldFseek = true;

    if ((updated_reg->fieldMask & MASK_DATANASCIMENTO) && strcmp(old_reg->dataNascimento, updated_reg->dataNascimento) != 0) {
        if (shouldFseek) fseek(file, registry_seek_start + offsets[6], SEEK_SET);
        binary_write_string(file, updated_reg->dataNascimento, 10);
        shouldFseek = false;
    }
    else shouldFseek = true;

    if ((updated_reg->fieldMask & MASK_SEXOBEBE) && old_reg->sexoBebe != updated_reg->sexoBebe) {
        if (shouldFseek) fseek(file, registry_seek_start + offsets[7], SEEK_SET);
        binary_write_char(file, updated_reg->sexoBebe);
        shouldFseek = false;
    }
    else shouldFseek = true;

    if ((updated_reg->fieldMask & MASK_ESTADOMAE) && strcmp(old_reg->estadoMae, updated_reg->estadoMae) != 0) {
        if (shouldFseek) fseek(file, registry_seek_start + offsets[8], SEEK_SET);
        binary_write_string(file, updated_reg->estadoMae, 2);
        shouldFseek = false;
    }
    else shouldFseek = true;

    if ((updated_reg->fieldMask & MASK_ESTADOBEBE) && strcmp(old_reg->estadoBebe, updated_reg->estadoBebe) != 0) {
        if (shouldFseek) fseek(file, registry_seek_start + offsets[9], SEEK_SET);
        binary_write_string(file, updated_reg->estadoBebe, 2);
        shouldFseek = false;
    }
    else shouldFseek = true;

    //Posiciona o cursor no fim do registro se o estadobebe não tiver sido escrito
    if (shouldFseek) fseek(file, registry_seek_start + REGISTRY_SIZE, SEEK_SET);

    //Libera a memória do registro anterior
    virtual_registry_delete(&old_reg);
}

/**
 *  Função de baixo nível que escreve um registro no disco
 *  OBS: o cursor deve estar posicionado corretamente antes de chamar esta função
 *  Parâmetros:
 *      FILE *file -> stream do arquivo binário com modo que permita escrita
 *      VirtualRegistry *reg_data -> registro a ser escrito
 *  Retorno: void
 */
void binary_write_registry(FILE *file, VirtualRegistry *reg_data) {
    if (file == NULL || reg_data == NULL) return;

    int cidadeMae_size, cidadeBebe_size;
    char *garbage_str;

    cidadeMae_size = strlen(reg_data->cidadeMae);
    cidadeBebe_size = strlen(reg_data->cidadeBebe);
    garbage_str = generate_garbage(calculate_variable_fields_garbage(cidadeMae_size+cidadeBebe_size));

    //Campos variáveis com tamanho
    binary_write_int(file, cidadeMae_size);
    binary_write_int(file, cidadeBebe_size);
    binary_write_string(file, reg_data->cidadeMae, cidadeMae_size);
    binary_write_string(file, reg_data->cidadeBebe, cidadeBebe_size);

    //Lixo
    binary_write_string(file, garbage_str, (int) strlen(garbage_str));
    free(garbage_str);
    
    //Campos estáticos
    binary_write_int(file, reg_data->idNascimento);
    binary_write_int(file, reg_data->idadeMae);
    binary_write_string(file, reg_data->dataNascimento, 10);
    binary_write_char(file, reg_data->sexoBebe);
    binary_write_string(file, reg_data->estadoMae, 2);
    binary_write_string(file, reg_data->estadoBebe, 2);
}


/**
 *  Função de baixo nível que lê um registro do disco
 *  OBS: o cursor deve estar posicionado corretamente antes de chamar esta função
 *  Parâmetros:
 *      FILE *file -> stream do arquivo binário
 *  Retorno: 
 *      VirtualRegistry* -> registro lido, ou NULL se o registro estiver deletado
 */
VirtualRegistry *binary_read_registry(FILE *file) {
    int cidadeMae_size, cidadeBebe_size, garbage_size;

    cidadeMae_size = binary_read_int(file);
    
    if (cidadeMae_size == -1) {
        fseek(file, -sizeof(int), SEEK_CUR);
        return NULL;
    }

    cidadeBebe_size = binary_read_int(file);
    garbage_size = calculate_variable_fields_garbage(cidadeMae_size+cidadeBebe_size);

    //Tenta alocar um registro na RAM
    VirtualRegistry *reg_data = virtual_registry_create();
    if (reg_data == NULL) {
        fprintf(stderr, "ERROR: unable to create VirtualRegistry @binary_read_registry()");
        return NULL;
    }

    //Campos variáveis
    reg_data -> cidadeMae = binary_read_string(file, cidadeMae_size);
    reg_data -> cidadeBebe = binary_read_string(file, cidadeBebe_size);

    //Ignora o lixo
    fseek(file, garbage_size, SEEK_CUR);
    
    //Campos estáticos
    reg_data->idNascimento = binary_read_int(file);
    reg_data->idadeMae = binary_read_int(file);
    reg_data->dataNascimento = binary_read_string(file, 10);
    reg_data->sexoBebe = binary_read_char(file);
    reg_data->estadoMae = binary_read_string(file, 2);
    reg_data->estadoBebe = binary_read_string(file, 2);

    return reg_data;
}

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

    char c;
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
    return VARIABLE_FIELDS_TOTAL_SIZE-strings_size_sum-8;
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