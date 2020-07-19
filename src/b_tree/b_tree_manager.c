#include "b_tree_manager.h"

#include <stdio.h>
#include <stdlib.h>

#include "binary_io.h"
#include "binary_b_tree.h"
#include "b_tree_header.h"
#include "b_tree_node.h"
#include "string_utils.h"
#include "debug.h"

#define NODE_SIZE 72

/*
	Struct auxiliar para a funcao recursiva de insercao na arvore-B.
	Permite o retorno de diversas informações no algoritmo recursivo de inserção
*/
struct _insert_answer {
	int key;
	int value;
	int RRN;
};

/*
	Struct que representa o gerenciador do arquivo de índices, usada para
	armazenar certas informações relacionadas ao arquivo, como os headers, 
	modo de abertura e rrn atual.
*/
struct _b_tree_manager {
	BTreeHeader *header;
	OPEN_MODE requested_mode;
	FILE *bin_file;
	int currRRN;
};


/*
	Funcao que cria um gerenciador da arvore-B, alocando memoria e definindo seus campos
	Parametros: nenhum
	Retorno:
		BTreeManager* . O gerenciador da arvore-B.
*/
BTreeManager *b_tree_manager_create(void) {
	//Tenta alocar memória
	BTreeManager *manager = malloc(sizeof(BTreeManager));
	if (manager == NULL) {
		DP("ERROR: not enough memory for BTreeManager @b_tree_manager_create()\n");
		return NULL;
	}

	//Define os valores iniciais
	manager -> header = NULL;
	manager -> bin_file = NULL;
	manager -> currRRN = -1;
	return manager;
}

/**
 *  Abre ou cria um arquivo binário, o qual será gerenciado pelo BTreeManager.
 *  Parâmetros:
 *      BTreeManager *manager -> instância do gerenciador
 *		char* bin_filename -> nome do arquivo (caminho completo)
 *      OPEN_MODE -> READ, CREATE ou MODIFY, indicando o modo de abertura do arquivo
 *  Retorno:
 *      OPEN_RESULT -> resultado da abertura (ler a documentação de OPEN_RESULT em open_mode.h)
 * 
 */
bool b_tree_manager_open(BTreeManager *manager, char* bin_filename, OPEN_MODE mode) {
	//Validação dos parâmetros, informando o código do problema por meio de um enum
    if (manager == NULL) return OPEN_INVALID_ARGUMENT;

	//Validação de parâmetros
    if (bin_filename == NULL) {
        DP("ERROR: (parameter) invalid null filename @b_tree_manager_open()\n");
        return OPEN_INVALID_ARGUMENT;
    }

	if (DEBUG && manager->bin_file != NULL) {
		DP("WARNING: opening new file before closing file already opened! @b_tree_manager_open()\n");
		return OPEN_INVALID_ARGUMENT;
	}

    //Vetor de conversão do enum modo para a string que o representa (READ = rb, CREATE = )
    static char* mode_to_str[] = {"rb", "wb+", "rb+"};

    //Guarda o modo de abertura para uso em outras funções
    manager->requested_mode = mode;

    //Abre o arquivo binário no modo selecionado (ler os modos)
    manager->bin_file = fopen(bin_filename, mode_to_str[mode]);

    //Se houver um erro na abertura do arquivo, retornar o erro por meio de um enum
    if (manager->bin_file == NULL) return OPEN_FAILED; 

    //Inicializa os headers com valores padrão (ou será usado para a escrita de um novo arquivo, ou substituído pelos headers do arquivo existente)
    manager->header = b_tree_header_create();
    
    //Se o modo for CREATE, ou seja, criar um novo arquivo, defina os headers com valores iniciais (RAM -> disco)
    if (mode == CREATE) {
        b_tree_header_write_to_bin(manager->header, manager->bin_file);
    } else { 
        //Se for outro modo, ou seja, o arquivo já existe, atualize o headers (disco -> RAM) e certifique-se de que o arquivo está consistente e não vazio
        b_tree_header_read_from_bin(manager->header, manager->bin_file);
        if (b_tree_header_get_status(manager->header) != '1') return OPEN_INCONSISTENT;

        if (mode == MODIFY) { 
			//Se houver intenção de modificar o arquivo, defina o status como inconsistente
            b_tree_header_set_status(manager->header, '0');
            b_tree_header_write_to_bin(manager->header, manager->bin_file);
        }
    }
    
    return OPEN_OK;
}

/**
 *  Escreve os headers no arquivo. Essa função deve ser usada o mínimo possível, uma vez
 *  que escrita a disco é custosa.
 *  Parâmetros:
 *      BTreeManager *manager -> gerenciador que possui os headers e o arquivo binário aberto em modo que permita a escrita
 *  Retorno: void
 */
void b_tree_manager_write_headers_to_disk(BTreeManager *manager) {
    //Validação de parâmetros
    if (manager == NULL) {
        DP("ERROR: (parameter) invalid null BTreeManager @b_tree_manager_write_headers_to_disk()\n");
        return;
    }

    //Valida o modo, impedindo tentativas de escrita no modo somente leitura
    if (manager->requested_mode == READ) {
        DP("ERROR: trying to write headers on a read-only BTreeManager @b_tree_manager_write_headers_to_disk()\n");
        return;
    }

    //Verifica se o manager não está em um estado inválido, isto é, quando o arquivo não está aberto ou os headers não estão definidos
    if (manager->header == NULL || manager->bin_file == NULL) {
        DP("ERROR: trying to write headers on a invalid BTreeManager state @b_tree_manager_write_headers_to_disk()\n");
        return;
    }

    //Escreve os headers no arquivo binário
    b_tree_header_write_to_bin(manager->header, manager->bin_file);
}

/**
 *  Lê os headers do arquivo. Essa função deve ser usada o mínimo possível, uma vez
 *  que leitura de disco é custosa.
 *  Parâmetros:
 *      BTreeManager *manager -> gerenciador que possui os headers e o arquivo binário aberto
 *  Retorno: void
 */
void b_tree_manager_read_headers_from_disk(BTreeManager *manager) {
    //Validação de parâmetros
    if (manager == NULL) {
        DP("ERROR: (parameter) invalid null BTreeManager @b_tree_manager_read_headers_from_disk()\n");
        return;
    }

    if (manager->header == NULL || manager->bin_file == NULL) {
        DP("ERROR: trying to read headers on a invalid BTreeManager state @b_tree_manager_read_headers_from_disk()\n");
        return;
    }

    b_tree_header_read_from_bin(manager->header, manager->bin_file);
}

/**
 *  Fecha o arquivo binário, limpando a memória de quaisquer estruturas auxiliares utilizadas
 *  Parâmetros:
 *      BTreeManager *manager -> gerenciador que possui o arquivo aberto
 *  Retorno: void
 */
void b_tree_manager_close(BTreeManager *manager) {
    //Verifica se o manager já foi deletado ou se o arquivo já foi fechado
    if (manager == NULL || manager->bin_file == NULL) return;
    
    if (manager->requested_mode != READ) {
		//Marca o arquivo como consistente. (OBS: não é necessário no caso da leitura, pois nenhuma modificação foi feita)
        b_tree_header_set_status(manager->header, '1');
		//Salva os headers no disco
        b_tree_manager_write_headers_to_disk(manager);
    }

	//Limpa a memória dos headers na RAM
    b_tree_header_free(&manager->header);

    //fecha o arquivo
    fclose(manager->bin_file);
	//Marca qua não existe arquivo aberto
    manager->bin_file = NULL;

	manager->currRRN = -1;
}


/*
	Funcao que desaloca a memoria de um gerenciador da btree. 
	Essa funcao NAO fecha o arquivo que estava sendo gerenciado
	Parametros:
		manager_ptr -> o endereco do gerenciador da btree
	Retorno:
		nao ha retorno 
*/
void b_tree_manager_free(BTreeManager **manager_ptr) {
	//Validação de parâmetros
	if (manager_ptr == NULL) {
		DP("ERROR: invalid parameter @b_tree_manager_free()\n");
		return;
	}
	#define manager (*manager_ptr)

	//Já foi liberado
	if (manager == NULL) return;

	b_tree_manager_close(manager);

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
static void _b_tree_manager_seek(BTreeManager *manager, int RRN) {
	//Validação de parâmetros
	if (manager == NULL) {
		DP("ERROR: invalid parameter @_b_tree_manager_seek()");
		return;
	}
	
	if (RRN < 0) {
		DP("ERROR: invalid given RRN @_b_tree_manager_seek()\n");
	}

	fseek(manager->bin_file, (RRN+1) * NODE_SIZE, SEEK_SET);
	manager->currRRN = RRN;
}

/*
	Funcao que le um node, precisa estar exatamente no comeco do node para funcionar
	Parametros:
		manager -> o gerenciador de node que tera' um node lido em seu binario
	Retorno:
		BTreeNode* -> O node lido
*/
BTreeNode *_b_tree_manager_read_current(BTreeManager *manager) {
	if (manager == NULL) {
		DP("ERROR: invalid parameter @_b_tree_manager_read_current()\n");
		return NULL;
	}

	manager->currRRN++;
	return binary_read_b_tree_node(manager->bin_file);
}

/*
	Funcao que le um node em um RRN dado
	Parametros:
		manager -> o gerenciador da arvore-B que tera' um nó lido em seu binario
		RRN -> o RRN do node a ser lido
	Retorno:
		BTreeNode* -> O node lido
*/
static BTreeNode *_read_node_at(BTreeManager *manager, int RRN) {
	if (manager == NULL) {
		DP("ERROR: invalid parameter @_read_node_at()\n");
		return NULL;
	}
	
	_b_tree_manager_seek(manager, RRN);				//faz o seek do RRN
	return _b_tree_manager_read_current(manager);	//le o nó e retorna
}

/*
	Funcao que escreve um node onde o ponteiro esta. Precisa estar exatamente no
	comeco de um RRN para ser eficiente
	Paramentros:
		manager -> o gerenciador de arvore-B que tera' um node escrito em seu binario
		node -> o node que sera escrito
	Retorno: void
*/
static void _write_current_node(BTreeManager *manager, BTreeNode *node) {
	if (manager == NULL || node == NULL) {
		DP("ERROR: invalid parameter @_write_current_node()\n");
		return;
	}

	binary_write_b_tree_node(manager->bin_file, node);		//escreve no binario
	manager->currRRN++;
}


/*
	Escreve um node em um RRN dado
	Paramentros:
		manager -> o gerenciador de arvore-B que tera' um node lido em seu binario
		RRN -> RRN do local onde sera escrito o node
		node -> o node que sera escrito
	Retorno: void
*/
static void _write_node_at(BTreeManager *manager, int RRN, BTreeNode *node) {
	if (manager == NULL || node == NULL) {
		DP("ERROR: invalid parameter @_write_node_at()\n");
		return;
	}

	_b_tree_manager_seek(manager, RRN);		//faz o seek
	_write_current_node(manager, node);		//escreve no binario
}

/*
	Faz a insercao recursiva de uma chave e de um valor em uma arvore-B
	Parametros:
		manager -> o gerenciador de arvore-B que sera inserido em seu binario
		nodeRRN -> o RRN do node que sera verificado para insercao
		key -> a chave que sera inserida
		value -> o valor que sera inserido
	Retorno:
		insert_answer. A struct que possui as informacoes de "volta" na recursao, para eventuais promocoes. ou com valores nulos, caso promocoes nao se facam necessarias
*/
insert_answer recursive_insert(BTreeManager *manager, int nodeRRN, int key, int value) {
	//Caso base: caso tenha passado do no folha.
	if (nodeRRN == -1) {
		insert_answer ans;
		ans.key = key;
		ans.value = value;
		ans.RRN = -1;
		return ans;
	}

	//le o node no RRN passado
	BTreeNode *node = _read_node_at(manager, nodeRRN);
	//pega o contador de itens no node
	int n = b_tree_node_get_n(node);
	//pega o proximo RRN no caminho pela arvore onde a chave melhor se encaixaria
	int nextRRN = b_tree_node_get_RRN_that_fits(node, key);
	insert_answer ans;
	
	//caso b_tree_node_get_RRN_that_fits() retorne -2, significa que a chave ja existe na arvore.
	//Assim, a funcao retorna valores nulos para nao haver insercao de chaves repetidas
	if (nextRRN == -2) {
		ans.key = -1;
		ans.value = -1;
		ans.RRN = -1;
		return ans;
	} 

	//chama a recursao
	ans = recursive_insert(manager, nextRRN, key, value);

	//caso a recursao retorne valores nulos (!= -1), significa que nao ha promocao para ser feita, entao nao faz nada.
	//caso retorne valores validos, a promocao precisa ser feita
	if (ans.key != -1) {
		//caso os vetores de itens nao estejam cheios, insere o item no node, e atualiza a struct de retorno com valores nulos, pois nao ha necessidade de promocao
		if (n < B_TREE_ORDER-1) {
			int pos = b_tree_node_sorted_insert_item(node, ans.key, ans.value);
			b_tree_node_insert_P(node, ans.RRN, pos+1);
			ans.key = -1;
			ans.value = -1;
			ans.RRN = -1;
		}
		//caso os vetores de item estejam cheios, faz a funcao de split 1-to-2 e seleciona o item pra promocao no node de nivel superior
		else {
			BTreeNode *new = b_tree_node_split_one_to_two(node, ans.key, ans.value, ans.RRN);
			//pega o primeiro item do node novo para promover, o item da direita, de acordo com a especificacao
			ans.key = b_tree_node_get_C(new, 0);
			ans.value = b_tree_node_get_Pr(new, 0);
			ans.RRN = b_tree_header_get_proxRRN(manager->header);
			
			//remove o item que sera promovido
			b_tree_node_remove_item(new, 0);
			
			//escreve o node novo
			_write_node_at(manager, ans.RRN, new);

			//incrementa o valor do proximo RRN no header
			b_tree_header_set_proxRRN(manager->header, H_INCREASE);

			//desaloca a memoria do node novo
			b_tree_node_free(new);
		}
		//atualiza o node antigo, caso tenha acontecido alguma alteracao
		_write_node_at(manager, nodeRRN, node);
	}

	//desaloca a memoria no node
	b_tree_node_free(node);
	
	return ans;
}

/*
	Faz a insercao de uma chave e um valor na arvore-B
	Parametros:
		manager -> o gerenciador de arvore-B que sera inserido em seu binario
		regIdNascimento -> a chave que sera inserida
		regRRN -> o valor que sera inserido
*/
void b_tree_manager_insert(BTreeManager *manager, int regIdNascimento, int regRRN) {
	if (manager == NULL) {
		return;
	}

	//pega o RRN do no raiz, para iniciar a busca do node para insercao
	int nodeRRN = b_tree_header_get_noRaiz(manager->header);
	//faz a insercao recursiva
	insert_answer ans = recursive_insert(manager, nodeRRN, regIdNascimento, regRRN);
	
	//caso a insercao recursiva retorne valores validos ate a essa funcao, significa que um novo no raiz precisa ser criado
	if (ans.key != -1) {
		int nextRRN = b_tree_header_get_proxRRN(manager->header);
		b_tree_header_set_noRaiz(manager->header, nextRRN);
		b_tree_header_set_nroNiveis(manager->header, H_INCREASE);
		b_tree_header_set_proxRRN(manager->header, H_INCREASE);

		//cria um novo no, para ser o no raiz
		BTreeNode *new = b_tree_node_create(b_tree_header_get_nroNiveis(manager->header));
		//inserte o item no no' raiz
		b_tree_node_sorted_insert_item(new, ans.key, ans.value);
		//insere os Ps no no' raiz
		b_tree_node_set_P(new, nodeRRN, 0);
		b_tree_node_set_P(new, ans.RRN, 1);
		
		//escreve o novo no' raiz
		_write_node_at(manager, nextRRN, new);
		b_tree_node_free(new);
	}

	b_tree_header_set_nroChaves(manager->header, H_INCREASE);

	return;
}

/*
	Funcao de busca na arvore-B
	Paramentros:
		manager-> o gerenciador de arvore-B que sera procurado em seu binario
		key -> a chave que sera procurada
	Retorno:
		pairIntInt. uma struct com dois valores int:
			first -> o valor atrelado a chave procurada. -1 caso a chave nao seja encontrada
			second -> o numero de acessos a disco necessarios para achar a chave

*/
pairIntInt b_tree_manager_search_for (BTreeManager *manager, int key) {
	pairIntInt p;
	if (manager == NULL) {
		p.first = -1;
		p.second = -1;
		return p;
	}

	p.second = 0;
	BTreeNode *node;
	//pega o no' raiz para iniciar a busca
	int nodeRRN = b_tree_header_get_noRaiz(manager->header);

	//caso nodeRRN seja -1, significa que a chave nao esta inserida na arvore-B
	while (nodeRRN != -1) {
		node = _read_node_at(manager, nodeRRN);
		p.second++;
		if (node == NULL)
			break;
		//pega o proximo RRN no caminho pela arvore onde a chave melhor se encaixaria.
		nodeRRN = b_tree_node_get_RRN_that_fits(node, key);

		//caso b_tree_node_get_RRN_that_fits() retorne -2, significa que a chave ja existe no node
		if (nodeRRN == -2) {
			//procura no node atual a chave, e a retorna
			for (int i = 0; i < B_TREE_ORDER; i++) {
				if (b_tree_node_get_C(node, i) == key) {
					int ans = b_tree_node_get_Pr(node, i);
					b_tree_node_free(node);
					p.first = ans;
					return p;
				}
			}
		}

		b_tree_node_free(node);
	}

	p.first = -1;
	return p;
}