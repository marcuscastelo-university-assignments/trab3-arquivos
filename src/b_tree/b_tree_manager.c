#include "b_tree_manager.h"

#include <stdio.h>
#include <stdlib.h>

#include "binary_io.h"
#include "binary_b_tree.h"
#include "b_tree_header.h"
#include "b_tree_node.h"
#include "string_utils.h"
#include "debug.h"

#define print_error(x) DP(x);

#define REG_SIZE 72

struct _insert_answer {
	int key;
	int value;
	int RRN;
};

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
		DP("ERROR: not enough memory for BTreeManager @b_tree_manager_create()\n");
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
		DP("ERROR: invalid parameter @b_tree_manager_delete()\n");
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

	int end = b_tree_header_get_proxRRN(manager->header);
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

*/
insert_answer recursive_insert(BTreeManager *manager, int nodeRRN, int idNascimento, int RRN) {
	if (nodeRRN == -1) {
		insert_answer ans;
		ans.key = idNascimento;
		ans.value = RRN;
		ans.RRN = -1;
		return ans;
	}

	BTreeNode *node = b_tree_manager_read_at(manager, nodeRRN);
	int n = b_tree_node_get_n(node);
	int nextRRN = b_tree_node_get_RRN_that_fits(node, idNascimento);
	insert_answer ans;

	if (nextRRN == -2) {
		ans.key = -1;
		ans.value = -1;
		ans.RRN = -1;
		return ans;
	} 

	ans = recursive_insert(manager, nextRRN, idNascimento, RRN);

	if (ans.key != -1) {
		if (n < B_TREE_ORDER-1) {
			b_tree_node_sorted_insert_item(node, idNascimento, RRN);
			ans.key = -1;
			ans.value = -1;
			ans.RRN = -1;
		}
		else {
			BTreeNode *new = b_tree_node_split_one_to_two(node, ans.key, ans.value, ans.RRN);
			ans.key = b_tree_node_get_C(new, 0);
			ans.value = b_tree_node_get_Pr(new, 0);
			ans.RRN = b_tree_header_get_proxRRN(manager->header);

			b_tree_node_remove_item(new, 0);

			b_tree_manager_write_at(manager, ans.RRN, new);

			b_tree_header_set_proxRRN(manager->header, H_INCREASE);
			b_tree_header_set_nroChaves(manager->header, H_INCREASE);

			b_tree_node_free(new);
		}
		
		b_tree_manager_write_at(manager, nodeRRN, node);
	}

	b_tree_node_free(node);
	
	return ans;
	// if (nivel == 1) {
	// 	if (n < B_TREE_ORDER-1) {
	// 		b_tree_node_sorted_insert_item(node, idNascimento, RRN);
	// 		b_tree_manager_write_at(manager, nodeRRN, node);

	// 		ans.key = -1;	
	// 		ans.value = -1;
	// 		ans.RRN = -1;
	// 	}
	// 	else {
	// 		BTreeNode *new = b_tree_node_split_one_to_two(node, idNascimento, RRN, -1);

	// 		ans.key = b_tree_node_get_C(new, 0);
	// 		ans.value = b_tree_node_get_Pr(new, 0);
	// 		ans.RRN = b_tree_header_get_proxRRN(manager->header);

	// 		b_tree_manager_write_at(manager, nodeRRN, node);
	// 		b_tree_manager_write_at(manager, ans.RRN, new);

	// 		b_tree_header_set_proxRRN(manager->header, H_INCREASE);
	// 		b_tree_header_set_nroChaves(manager->header, H_INCREASE);

	// 		b_tree_node_remove_item(node, 0);
	// 		b_tree_node_free(new);
	// 	}

	// 	b_tree_node_free(node);
	// 	return ans;
	// }

	// else {
	// 	int nextRRN = b_tree_node_get_RRN_that_fits(node, idNascimento);
	// 	BTreeNode *nextNode = b_tree_manager_read_at(manager, nextRRN);
	// 	insert_answer ans = recursive_insert(manager, nextNode, nextRRN, idNascimento, RRN);
	// 	if (ans.key >= 0) {
	// 		if (n < B_TREE_ORDER-1) {
	// 			int pos = b_tree_node_sorted_insert_item(node, ans.key, ans.value);
	// 			b_tree_node_insert_P(node, ans.RRN, pos+1);
	// 			b_tree_manager_write_at(manager, nodeRRN, node);

	// 			ans.key = -1;	
	// 			ans.value = -1;
	// 			ans.RRN = -1;
	// 		}
	// 		else {
	// 			//todo split no pai
	// 		}
	// 	}

	// 	free(nextNode);
	// 	return ans;
	// }
}

/*

*/
void b_tree_manager_insert(BTreeManager *manager, int idNascimento, int RRN) {
	if (manager == NULL) {
		return;
	}

	int nodeRRN = b_tree_header_get_noRaiz(manager->header);
	insert_answer ans = recursive_insert(manager, nodeRRN, idNascimento, RRN);

	if (ans.key != -1) {
		int nextRRN = b_tree_header_get_proxRRN(manager->header);
		b_tree_header_set_noRaiz(manager->header, nextRRN);
		b_tree_header_set_nroChaves(manager->header, H_INCREASE);
		b_tree_header_set_nroNiveis(manager->header, H_INCREASE);
		b_tree_header_set_proxRRN(manager->header, H_INCREASE);

		BTreeNode *new = b_tree_node_create(b_tree_header_get_nroNiveis(manager->header));
		b_tree_node_sorted_insert_item(new, ans.key, ans.value);
		b_tree_node_set_P(new, nodeRRN, 0);
		b_tree_node_set_P(new, ans.RRN, 1);

		b_tree_manager_write_at(manager, nextRRN, new);
		b_tree_node_free(new);
	}

	return;
}

/*

*/
int b_tree_manager_search_for (BTreeManager *manager, int idNascimento) {
	if (manager == NULL) {
		return -1;
	}

	BTreeNode *node;
	int nodeRRN = b_tree_header_get_noRaiz(manager->header);

	while (nodeRRN != -1) {
		node = b_tree_manager_read_at(manager, nodeRRN);
		if (node == NULL)
			break;
		//i = 0, 1, 2, 3,  4
		//C = 2, 4, 7, 8, 10
		//Pr= 1, 2, 3, 4,  5, 6

		nodeRRN = b_tree_node_get_RRN_that_fits(node, idNascimento);

		if (nodeRRN == -2) {
			for (int i = 0; i < B_TREE_ORDER; i++) {
				if (b_tree_node_get_C(node, i) == idNascimento) {
					int ans = b_tree_node_get_Pr(node, i);
					b_tree_node_free(node);
					return ans;
				}
			}
		}

		b_tree_node_free(node);
	}

	return -1;
}