#ifndef __STRING_UTILS__H__
#define __STRING_UTILS__H__

#include "open_mode.h"

void binarioNaTela(const char *nomeArquivoBinario);
char *_csv_registry_token(char *string);
int is_string_empty(char *string);
char *parse_sexoBebe_for_print (char sexoBebe);
char *parse_string_for_print(char *value);
void scan_quote_string(char **str_ptr);
void open_result_print_message(OPEN_RESULT open_result);

#endif  //!__STRING_UTILS__H__
