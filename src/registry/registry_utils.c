#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "binary_io.h"
#include "registry_utils.h"
#include "string_utils.h"
#include "debug.h"


/*
	Essa funcao cria uma string vazia com lixo, para variaveis de tamanho fixo. O primeiro caracter e' '\0' e os outros sao '$'
	Parametros:
		size -> tamanho da string a ser criada
	Retorno:
		uma string vazia com lixo 
*/
void static_value_fill_with_garbage(char **value_ptr, int expectedSize) {
	#define value (*value_ptr)
	if (value_ptr == NULL || expectedSize < 0) {
		DP("ERROR: invalid parameters @static_value_fill_with_garbage\n");
		return;
	}

	int valueSize = (value == NULL)? 0 : strlen(value);

	if (valueSize == expectedSize) return;

    char *garbage = generate_garbage(expectedSize - valueSize);

	char *oldValue = value;

	if (valueSize == 0) {
		value = garbage;
		value[0] = '\0';
	} else {
		value = calloc(expectedSize, sizeof(char));
		strcat(value, oldValue);
		strcat(value, garbage);
		free(garbage);
	}
	free(oldValue);

	#undef value

	return;
}


/*
	Essa funcao trata de valores invalidos no registro, para a escrita ser bem sucedida
	Parametros:
		registry -> o registro para ser tratado
	Retorno:
		nao ha retorno
*/
void registry_prepare_for_write(VirtualRegistry *registry) {
	if (registry == NULL)
		return;

	if (registry->idadeMae <= 0)		//caso a idadeMae tenha valor 0 ou inválido
		registry->idadeMae = -1;		//muda para o valor que representa a idadeMae ignorada

	if (registry->cidadeMae == NULL)	
		registry->cidadeMae = strdup("");	//escreve uma string vazia para nao deixar nulo

	if (registry->cidadeBebe == NULL)
		registry->cidadeBebe = strdup("");

	if (registry->idNascimento < -1)	//caso o idNascimento tenha um valor invalido	
		registry->idNascimento = -1;	//muda para o valor que representa idNascimento invalido

	if (registry->sexoBebe < '0' || registry->sexoBebe > '2') //caso sexoBebe tenha valor invalido, escreve o valor que representa "IGNORADO"
		registry->sexoBebe = '0';

	static_value_fill_with_garbage(&registry->dataNascimento, 10);
	static_value_fill_with_garbage(&registry->estadoBebe, 2);
	static_value_fill_with_garbage(&registry->estadoMae, 2);
}


/*
	Funcao semelhante a strcmp(), mas esta aceita NULL. Criada para poupar tempo tratando NULL.
	Parametros:
		str1 -> string para ser comparada
		str2 -> string para ser comparada
	Retorno:
		bool. true caso as strings sejam ambas nulas ou iguais. false caso apenas uma seja nula, ou sejam diferentes
*/
bool compare_string_field (char *str1, char *str2) {
	if (str1 == NULL && str2 == NULL)
		return true;
	
	if (str1 == NULL || str2 == NULL)
		return false;

	return (strcmp(str1, str2) == 0) ? true : false;
}