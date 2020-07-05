#include "b_tree_manager.h"

#include <stdio.h>
#include <stdlib.h>

#include "binary_io.h"
#include "binary_b_tree.h"
#include "b_tree_header.h"
#include "b_tree_node.h"
#include "string_utils.h"

#define print_error(x) fprintf(stderr, x);

#define REG_SIZE 72

/*
	Struct que representa o gerenciador de registros, usada para debug
	e para diferenciacao de nivel de complexidade no programa
*/
struct _b_tree_manager {
	BTHeader *header;			//Referência ao header gerenciado pelo DataManager
	FILE *bin_file;			//ponteiro do arquivo aberto e tendo seus registros gerenciados
	int currRRN;			//RRN atual do ponteiro
};


/*
	Funcao que cria um gerenciador de registros, alocando memoria e definindo seus campos
	Parametros:
		bin_file -> ponteiro do arquivo que tera' seus registros gerenciados.
	Retorno:
		BTreeManager* . O gerenciador de registros.
*/
BTreeManager *b_tree_manager_create(FILE *bin_file, BTHeader *header) {
	//Tenta alocar memória
	BTreeManager *manager = malloc(sizeof(BTreeManager));
	if (manager == NULL) {
		fprintf(stderr, "ERROR: not enough memory for BTreeManager @b_tree_manager_create()\n");
		return NULL;
	}

	//Define os valores iniciais
	manager -> header = header;
	manager -> bin_file = bin_file;
	manager -> currRRN = -1;
	return manager;
}


/*
	Funcao que desaloca a memoria de um gerenciador de registros. 
	Essa funcao NAO fecha o arquivo que estava sendo gerenciado
	Parametros:
		manager_ptr -> o endereco do gerenciador de registros
	Retorno:
		nao ha retorno 
*/
void b_tree_manager_delete(BTreeManager **manager_ptr) {
	//Validação de parâmetros
	if (manager_ptr == NULL) {
		fprintf(stderr, "ERROR: invalid parameter @b_tree_manager_delete()\n");
		return;
	}
	#define manager (*manager_ptr)

	//Já foi liberado
	if (manager == NULL) return;

	//Redefine os valores ao padrão inicial
	manager -> bin_file = NULL;
	manager -> currRRN = -1;
	free(manager);
	manager = NULL;
	#undef manager
}


/*
	Funcao para fazer seek no arquivo sendo gerenciado
	Parametros:
		manager -> o gerenciador que contem o arquivo onde sera feito o seek
		RRN -> RRN para onde o ponteiro sera levado
	Retorno: void
*/
void b_tree_manager_seek(BTreeManager *manager, int RRN) {
	//Validação de parâmetros
	if (manager == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_seek()");
		return;
	}
	
	if (RRN < 0) {
		print_error("ERROR: invalid given RRN @b_tree_manager_seek()\n");
	}

	fseek(manager->bin_file, (RRN+1) * REG_SIZE, SEEK_SET);
	manager->currRRN = RRN;
}

/*
	Funcao que faz fseek para o primeiro registro (RRN = 0)
	Parametros:
		manager -> o gerenciador que contem o arquivo onde sera feito o seek
	Retorno: void
*/
void b_tree_manager_seek_first(BTreeManager *manager) {
	//Validação de parâmetros
	if (manager == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_seek_first\n");
		return;
	}

	b_tree_manager_seek(manager, 0);
}


/*
	Funcao que faz fseek para o ultimo registro
	Parametros:
		manager -> o gerenciador que contem o arquivo onde sera feito o seek
	Retorno: void
*/
void b_tree_manager_seek_end(BTreeManager *manager) {
	if (manager == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_seek_end\n");
		return;
	}

	int end = reg_header_get_next_RRN(manager->header);
	b_tree_manager_seek(manager, end);
}

/*
	Funcao que le um registro, precisa estar exatamente no comeco do registro para funcionar
	Parametros:
		manager -> o gerenciador de registro que tera' um registro lido em seu binario
	Retorno:
		BTreeNode* -> O registro lido. NULL caso o registro tenha sido removido
*/
BTreeNode *b_tree_manager_read_current(BTreeManager *manager) {
	if (manager == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_read_current()\n");
		return NULL;
	}

	if (b_tree_manager_is_current_deleted(manager) == true) {	//verifica se o registro foi removido
		b_tree_manager_jump_node(manager, FRONT);					//caso tenha sido removido, pula para o proximo registro
		return NULL;
	}

	manager->currRRN++;
	return binary_read_b_tree_node(manager->bin_file);
}


/*
	Funcao que le um registro em um RRN dado
	Parametros:
		manager -> o gerenciador de registro que tera' um registro lido em seu binario
		RRN -> o RRN do registro a ser lido
	Retorno:
		BTreeNode* -> O registro lido. NULL caso o registro tenha sido removido
*/
BTreeNode *b_tree_manager_read_at(BTreeManager *manager, int RRN) {
	if (manager == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_read_at()\n");
		return NULL;
	}
	
	b_tree_manager_seek(manager, RRN);			//faz o seek do RRN
	return b_tree_manager_read_current(manager);	//le o registro e retorna
}


/*
	Funcao que escreve um registro onde o ponteiro esta. Precisa estar exatamente no 
	comeco de um RRN para ser eficiente
	Paramentros:
		manager -> o gerenciador de registro que tera' um registro escrito em seu binario
		node -> o registro que sera escrito
	Retorno: void
*/
void b_tree_manager_write_current(BTreeManager *manager, BTreeNode *node) {
	if (manager == NULL || node == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_write_current()\n");
		return;
	}

	registry_prepare_for_write(node);					//faz o tratamento de campos invalidos
	binary_write_b_tree_node(manager->bin_file, node);		//escreve no binario
	manager->currRRN++;
}


/*
	Escreve um registro em um RRN dado
	Paramentros:
		manager -> o gerenciador de registro que tera' um registro lido em seu binario
		RRN -> RRN do local onde sera escrito o registro
		node -> o registro que sera escrito
	Retorno: void
*/
void b_tree_manager_write_at(BTreeManager *manager, int RRN, BTreeNode *node) {
	if (manager == NULL || node == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_write_at()\n");
		return;
	}

	b_tree_manager_seek(manager, RRN);				//faz o seek
	b_tree_manager_write_current(manager, node);	//escreve no binario
}


/*
	Funcao que deleta o registro onde o ponteiro esta'
	Precisa estar exatamente no comeco do registro para ser eficiente
	Parametros:
		manager -> o gerenciador de registro que tera' o registro deletado em seu binario
	Retorno: void
*/
void b_tree_manager_delete_current (BTreeManager *manager) {
	if (manager == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_delete_current()\n");
		return;
	}

	binary_write_int(manager->bin_file, -1);	//escreve o indicador de registro deletado: -1
	
	fseek(manager->bin_file, REG_SIZE-sizeof(int), SEEK_CUR); //faz o seek para ir para o final do registro

	manager->currRRN++;
}


/*
	Deleta um registro em um RRN dado
	Paramentros:
		manager -> o gerenciador de registro que tera' um registro deletado em seu binario
		RRN -> RRN do local onde sera deletado o registro
	Retorno: void
*/
void b_tree_manager_delete_at (BTreeManager *manager, int RRN) {
	if (manager == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_delete_at()\n");
		return;
	}

	b_tree_manager_seek(manager, RRN);		//faz o seek do RRN
	b_tree_manager_delete_current(manager);	//deleta o registro
}


/*
	Verifica se o registro onde o ponteiro esta foi deletado
	Precisa estar no comeco do registro para ser eficiente
	Parametros:
		manager -> o gerenciador de registros que tera' seu registro verificado
	Retorno:
		bool.
		true, caso o registro esteja deletado
		false, caso contrario 
*/
bool b_tree_manager_is_current_deleted (BTreeManager *manager) {
	if (manager == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_is_current_deleted()\n");
		return false;
	}

	int N = binary_read_int(manager->bin_file);

	fseek(manager->bin_file, -sizeof(int), SEEK_CUR); //volta para o inicio do registro, para a posicao antes de ler o numero
	
	return (N == -1) ? true : false;
}


/*
	Funcao que pula os bytes de um registro (128 bytes) em uma determinada direcao
	Parametros:
		manager -> o gerenciador de registros que tera seu ponteiro alterado
		direction: 
				  -> FRONT : pula +128 bytes (expands to +1)
				  -> BACK  : pula -128 bytes (expands to -1)
	Retorno: void
*/
void b_tree_manager_jump_node (BTreeManager *manager, int direction) {
	if (manager == NULL) {
		print_error("ERROR: invalid parameter @b_tree_manager_jump_node()\n");
		return;
	}

	fseek(manager->bin_file, REG_SIZE*direction, SEEK_CUR); //faz o seek para de +-128 bytes, pulando um registro pra frente ou pra tras 
	
	manager->currRRN += direction;
}