#include "b_tree_node.h"

#include <stdio.h>
#include <stdlib.h>

/**
 *  Struct que define o TAD BTreeNode
 *  indica em si as características do nó
 *  conforme comentado abaixo
 */
struct _b_tree_node {
    int nivel;              //Nível do nó na árvore (1 indica que é uma folha)
    int n;                  //Quantidade atual de chaves no nó    
    int C[B_TREE_ORDER-1];  //Vetor de chaves (chaves não inseridas são representadas por -1)
    int Pr[B_TREE_ORDER-1]; //Vetor de valores para cada chave (valores não inserido são -1)
    int P[B_TREE_ORDER];    //Vetor de "pointers" para os filhos do nó (indocam o RRN do nó filho)
};


/*
    Essa funcao cria um node da btree.
    Parametros:
        nivel -> o nivel do node na btree
    Retorno:
        BtreeNode* . O node criado.
*/
BTreeNode* b_tree_node_create (int nivel) {
    BTreeNode *bTreeNode = (BTreeNode*) malloc (sizeof(BTreeNode));
    if (bTreeNode == NULL)
        return NULL;

    b_tree_node_set_nivel(bTreeNode, nivel);
    bTreeNode->n = 0;

    //preenche os vetores com -1
    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        bTreeNode->C[i] = -1;
        bTreeNode->Pr[i] = -1;
    }

    for (int i = 0; i < B_TREE_ORDER; i++) {
        bTreeNode->P[i] = -1;
    }

    return bTreeNode; 
}

/*
    Desaloca a memoria de um node
    Parametros:
        node -> o node para ser desalocado
*/
void b_tree_node_free (BTreeNode *node) {
    if (node == NULL)
        return;

    free(node);

    return;
}

//////SET FUNCTIONS//////
/*
    Seta o nivel de um node
    Parametros:
        node -> o node para ser modificado
        nivel -> o nivel para setar no node
*/
void b_tree_node_set_nivel (BTreeNode *node, int nivel) {
    if (node == NULL)
        return;

    node->nivel = nivel;

    return;
}

/*
    Insere, de maneira ordenada, um item (o par C e Pr) em seus vetores no node
    Parametros:
        node -> o node para inserir
        C -> o valor C da arvore
        Pr -> o valor Pr da arvore
    Retorno:
        int. A posicao em que o item foi inserido em seus vetores. -1 caso a operacao nao possa ser realizada
*/
int b_tree_node_sorted_insert_item (BTreeNode *node, int C, int Pr) {
    if (node == NULL)
        return -1;
    
    if (C == -1)
        return -1;

    if (node->n == B_TREE_ORDER-1)
        return -1;

    int pos = -1;

    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        //se for uma posicao inutilizada, insere nessa posicao
        if (node->C[i] == -1) {
            node->C[i] = C;
            node->Pr[i] = Pr;
            pos = i;
            break;
        }

        //se encontrar a posicao para inserir, move o restante do vetor para inserir
        else if (node->C[i] > C) {
            for (int j = B_TREE_ORDER-3; j >= i; j--) {
                node->C[j+1] = node->C[j];
                node->Pr[j+1] = node->Pr[j];
            }

            node->C[i] = C;
            node->Pr[i] = Pr;
            pos = i;
            break;
        }
    }

    node->n++;

    return pos;
}


/*
    Seta um item em um node, dada uma posicao.
    Parametros:
        node -> o node para modificar
        C -> o valor C da arvore
        Pr -> o valor Pr da arvore
        position -> a posicao a ser modificada
*/
void b_tree_node_set_item (BTreeNode *node, int C, int Pr, int position) {
    if (node == NULL)
        return;

    if (position >= B_TREE_ORDER-1 || position < 0)
        return;

    if (node->C[position] == -1) node->n++; //aumenta o contador caso seja um item novo (o item anterior seja -1). caso esteja substituindo um item, nao aumenta o contador
    node->C[position] = C;
    node->Pr[position] = Pr;

    return;
}


/*
    Insere P em um node, dada uma posicao
    Parametros:
        node -> o node para inserir
        P -> o valor P da arvore
        position -> a posicao a ser inserida
*/
void b_tree_node_insert_P (BTreeNode *node, int P, int position) {
    if (node == NULL)
        return;

    for (int i = B_TREE_ORDER-2; i >= position; i--) {
        node->P[i+1] = node->P[i]; 
    }

    node->P[position] = P;

    return;
}

/*
    Seta P em um node, dada uma posicao
    Parametros:
        node -> o node para modificar
        P -> o valor P da arvore
        position -> a posicao a ser modificada
*/
void b_tree_node_set_P (BTreeNode *node, int P, int position) {
    if (node == NULL)
        return;

    if (position >= B_TREE_ORDER || position < 0)
        return;

    node->P[position] = P;

    return;
}

//////GET FUNCTIONS//////
/*
    Retorna o nivel do node
    Parametros:
        node -> o node ser lido
    Retorno:
        int. o nivel do node
*/
int b_tree_node_get_nivel (BTreeNode *node) {
    if (node == NULL)
        return -1;

    return node->nivel;
}

/*
    Retorna o N do node
    Parametros:
        node -> o node ser lido
    Retorno:
        int. o N do node
*/
int b_tree_node_get_n (BTreeNode *node) {
    if (node == NULL)
        return -1;

    return node->n;
}

/*
    Retorna o C do node dada uma posicao
    Parametros:
        node -> o node ser lido
        position -> a posicao para ler
    Retorno:
        int. o C do node nessa posicao
*/
int b_tree_node_get_C (BTreeNode *node, int position) {
    if (node == NULL)
        return -1;

    if (position >= B_TREE_ORDER-1 || position < 0)
        return -1;

    return node->C[position];
}

/*
    Retorna o Pr do node dada uma posicao
    Parametros:
        node -> o node ser lido
        position -> a posicao para ler
    Retorno:
        int. o Pr do node nessa posicao
*/
int b_tree_node_get_Pr (BTreeNode *node, int position) {
    if (node == NULL)
        return -1;

    if (position >= B_TREE_ORDER-1 || position < 0)
        return -1;

    return node->Pr[position];
}

/*
    Retorna o P do node dada uma posicao
    Parametros:
        node -> o node ser lido
        position -> a posicao para ler
    Retorno:
        int. o P do node nessa posicao
*/
int b_tree_node_get_P (BTreeNode *node, int position) {
    if (node == NULL)
        return -1;

    if (position >= B_TREE_ORDER || position < 0)
        return -1;    
    
    return node->P[position];
}

//////REM FUNCTIONS//////
/*
    Remove um item de um node dada uma posicao
    Parametros:
        node -> o node parater seu item removido
        position -> a posicao do node que sera removido
*/
void b_tree_node_remove_item (BTreeNode *node, int position) {
    if (node == NULL)
        return;

    if (position >= B_TREE_ORDER-1 || position < 0)
        return;
        
    if (node->C[position] != -1) node->n--; //caso seja um node valido, diminui o contador de itens

    for (int i = position; i < B_TREE_ORDER-1-1; i++) {
        node->C[i] = node->C[i+1];
        node->Pr[i] = node->Pr[i+1];
    }

    node->C[B_TREE_ORDER-2] = -1;
    node->Pr[B_TREE_ORDER-2] = -1;

    return;
}

/*
    Remove um P em um node, dada uma posicao
    Parametros: 
        node -> o node que tera seu P removido
        position -> a posicao do P que sera removido
*/
void b_tree_node_remove_P (BTreeNode *node, int position) {
    if (node == NULL)
        return;

    if (position >= B_TREE_ORDER || position < 0)
        return;

    for (int i = position; i < B_TREE_ORDER-1; i++) {
        node->P[i] = node->P[i+1];
    }

    node->P[B_TREE_ORDER-1] = -1;

    return;
}

////////////
/*
    Essa funcao verifica o proximo node no caminho de procura pelo node certo de uma determinada chave. Ou se o node atual ja possui essa chave.
    Funcao utilizada na busca e na insercao para determinar o caminho a ser percorrido pela arvore
    Parametros:
        node -> o node que sera lido
        key -> a chave que sera comparada
    Retorno:
        int. o proximo RRN para ser lido na arvore. ou -1, caso haja erro na operacao. ou -2, caso a chave ja pertenca ao node 
*/
int b_tree_node_get_RRN_that_fits (BTreeNode *node, int key) {
    if (node == NULL || key < 0)
        return -1;

    int nodeRRN = 0;

    //flag para verificar se foi encontrado um lugar para melhor encaminhar a chave antes de terminar o vetor de itens
    bool enteredBeforeNodeEnd = false;
    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        int C = b_tree_node_get_C(node, i);

        if (C == -1 || key < C) {
            nodeRRN = b_tree_node_get_P(node, i);
            enteredBeforeNodeEnd = true;
            break;
        }

        //se a chave pertender ao node, retorna -2
        else if (key == C) {
            nodeRRN = -2;
            enteredBeforeNodeEnd = true;
            break;
        }  
    }

    //Se não tiver entrado ainda (já chegamos no fim do nó), força a entrada à direita
    if (!enteredBeforeNodeEnd)
        nodeRRN = b_tree_node_get_P(node, B_TREE_ORDER-1);

    return nodeRRN;
}

/*
    Faz uma insercao ordenada em um determinado array
    Parametros:
        arr -> o endereco do array que recebera o valor
        size -> o tamanho do array
        value -> o valor a ser inserido
    Retorno:
        int. a posicao onde foi inserido
*/
int insertion_sort_insert_in_array (int *arr, int size, int value) {
    for (int i = 0; i < size-1; i++) {
        if (value < arr[i]) {
            for (int j = size-2; j >= i; j--) {
                arr[j+1] = arr[j];
            }

            arr[i] = value;
            return i;
        }
    }

    arr[size-1] = value;
    return size-1;
}

/*
    Funcao que faz o split 1-to-2 de um node para a arvore. Por questoes de genericidade, mantem o item que sera promovido no node, 
    deixando a desicao de promocao para a funcao que chamar esta funcao
    Parametros:
        old -> o node que sera dividido
        C -> o C que sera inserido
        Pr -> o Pr que sera inserido
        P -> o P que sera inserido
    Retorno:
        BTreeNode* . o node novo apos a divisao
*/
BTreeNode *b_tree_node_split_one_to_two(BTreeNode *old, int C, int Pr, int P) {
	BTreeNode *new = b_tree_node_create(b_tree_node_get_nivel(old)); //cria um node com o mesmo nivel do antigo

    //aloca os vetores para fazer as insercoes devidas de C, Pr, e P.
    int *newC = (int*) malloc (sizeof(int) * B_TREE_ORDER);
    int *newPr = (int*) malloc (sizeof(int) * B_TREE_ORDER);
    int *newP = (int*) malloc (sizeof(int) * B_TREE_ORDER+1);

    //copia os valores do old
    for (int i = 0; i < B_TREE_ORDER-1; i++) {
        newC[i] = b_tree_node_get_C(old, i);
        newPr[i] = b_tree_node_get_Pr(old, i);
    }
    for (int i = 0; i < B_TREE_ORDER; i++) {
        newP[i] = b_tree_node_get_P(old, i);
    }
    
    //insere C na posicao devida
    int position = insertion_sort_insert_in_array(newC, B_TREE_ORDER, C);
    //insere Pr no mesmo valor de posicao que C foi inserido
    for (int i = B_TREE_ORDER-2; i >= position; i--) {
        newPr[i+1] = newPr[i];
    }
    newPr[position] = Pr;

    //insere P na posicao+1 do valor onde P foi inserido, para facilitar de acordo com as especificacoes
    for (int i = B_TREE_ORDER-1; i >= position+1; i--) {
        newP[i+1] = newP[i];
    }
    newP[position+1] = P;

    //seta os itens necessarios no node antigo
    for (int i = 0; i < B_TREE_ORDER/2; i++) {
        b_tree_node_set_item(old, newC[i], newPr[i], i);
    }
    //remove os itens desnecessarios do node antigo
    for (int i = B_TREE_ORDER/2; i < B_TREE_ORDER-1; i++) {
        b_tree_node_remove_item(old, B_TREE_ORDER/2);
    }

    //seta os itens no node novo (nao ha necessidade de remocao, pois o node e' novo e todos os seus valores ja eram nulos)
    for (int i = B_TREE_ORDER/2; i < B_TREE_ORDER; i++) {
        b_tree_node_sorted_insert_item(new, newC[i], newPr[i]);
    }

    //seta os RRNs necessarios no node antigo 
    for (int i = 0; i < B_TREE_ORDER/2+1; i++) {
        b_tree_node_set_P(old, newP[i], i);
    }
    //remove os RRNS desncessarios do node antigo
    for (int i = B_TREE_ORDER/2+1; i < B_TREE_ORDER; i++) {
        b_tree_node_set_P(old, -1, i);
    }
    
    //seta os RRNs no node novo
    int start = B_TREE_ORDER/2+1;
    for (int i = start; i < B_TREE_ORDER+1; i++) {
        b_tree_node_set_P(new, newP[i], i-start);
    }

    //desaloca a memoria dos vetores auxiliares
    free(newC);
    free(newPr);
    free(newP);

    return new; 
}