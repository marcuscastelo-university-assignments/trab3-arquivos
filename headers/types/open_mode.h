#ifndef __OPEN_MODE__H__
#define __OPEN_MODE__H__

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
 *  OPEN_INVALID_ARGUMENT
 */
typedef enum {
    OPEN_OK = 0, OPEN_FAILED = 1, OPEN_INCONSISTENT = 2, OPEN_INVALID_ARGUMENT = 8
} OPEN_RESULT;

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

#endif  //!__OPEN_MODE__H__