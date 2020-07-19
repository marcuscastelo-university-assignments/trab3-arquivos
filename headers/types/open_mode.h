#ifndef __OPEN_MODE__H__
#define __OPEN_MODE__H__

#include "debug.h"

/**
 *  Enum que determina os modos de abertura de arquivo e gerenciamento de status
 *  READ -> abre em rb e não mexe em nenhum byte do arquivo
 *  CREATE -> abre em wb+, criando um novo arquivo, definindo os headers com valores padrão e preenchendo seu lixo (só é feito na criação). Também define o status como '0' até que se finalizem as operações
 *  MODIFY -> abre o arquivo em rb+ para que seja possível fazer atualizações, deleções, etc... Abre o arquivo como se fosse READ, mas define o status como '0'
 */
typedef enum {
    READ = 0, CREATE = 1, MODIFY = 2
} OPEN_MODE;

/**
 *  Enum que indica o resultado da abertura do arquivo
 *  OPEN_OK -> indica que não houve problema na abertura do arquivo
 *  OPEN_FAILED -> Indica a necessidade de exibir a mensagem de erro "Falha no processamento do arquivo.", de acordo com as especificações do trabalho
 *  OPEN_INCONSISTENT ->  Indica que o arquivo está inconsistente, exibe "Falha no processamento do arquivo.", de acordo com as especificações do trabalho
 *  OPEN_INVALID_ARGUMENT -> Indica que houve um erro por parte do programador e um parâmetro incorreto foi passado
 */
typedef enum {
    OPEN_OK = 0, OPEN_FAILED = 1, OPEN_INCONSISTENT = 2, OPEN_INVALID_ARGUMENT = 8
} OPEN_RESULT;



#endif  //!__OPEN_MODE__H__