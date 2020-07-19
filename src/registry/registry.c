#include "registry.h"
#include "string_utils.h"
#include "registry_utils.h"
#include "binary_registry.h"


#include <stdlib.h>
#include <string.h>
#include "debug.h"

/**
 *  Função usada para criar cópias de registros,
 *  faz a cópia profunda de membros encapsulados.
 *  Parâmetros:
 *      VirtualRegistry *base -> registro que servirá de base para a cópia
 *  Retorno: 
 *      VirtualRegistry* -> novo registro criado com as informações copiadas do 'base'
 * 
 */
VirtualRegistry *virtual_registry_create_copy(VirtualRegistry *base) {
    VirtualRegistry *reg_data = virtual_registry_create();
    
    if (reg_data == NULL) {
        DP("ERROR: Insuficient memory on virtual_registry_create_copy()\n");
        return NULL;
    }
    
    if (base->cidadeBebe != NULL) reg_data->cidadeBebe = strdup(base->cidadeBebe);
    if (base->cidadeMae != NULL) reg_data->cidadeMae = strdup(base->cidadeMae);
    if (base->dataNascimento != NULL) reg_data->dataNascimento = strdup(base->dataNascimento);
    if (base->estadoBebe != NULL) reg_data->estadoBebe = strdup(base->estadoBebe);
    if (base->estadoMae != NULL) reg_data->estadoMae = strdup(base->estadoMae);
    reg_data->idadeMae = base->idadeMae;
    reg_data->idNascimento = base->idNascimento;
    reg_data->sexoBebe = base->sexoBebe;
    reg_data->fieldMask = base->fieldMask;

    return reg_data;
}

/**
 *   Funçao que aloca uma struct que representa o registro do tipo cheio (Ler tipos de registro na struct do .h).
 *   Parametros:
 *       nao há parametros
 *   Retorno:
 *       VirtualRegistry* -> Um ponteiro da struct criada 
 */
VirtualRegistry *virtual_registry_create() {
    return virtual_registry_create_masked(MASK_ALL);
}

/**
 *  Funçao que aloca uma struct que representa o registro do tipo mascarado (Ler tipos de registro na struct do .h).
 *  Parametros:
 *      RegistryMask mask -> indica quais campos devem ser usados para busca/atualização e quais devem ser ignorados
 *  Retorno:
 *      VirtualRegistry* -> Um ponteiro da struct criada 
 */
VirtualRegistry *virtual_registry_create_masked(RegistryFieldsMask mask) {
    VirtualRegistry *reg_data = malloc(sizeof(VirtualRegistry));
    
    if (reg_data == NULL) {
        DP("ERROR: Insuficient memory on virtual_registry_create()\n");
        return NULL;
    }

    reg_data->cidadeBebe = NULL;
    reg_data->cidadeMae = NULL;
    reg_data->dataNascimento = NULL;
    reg_data->estadoBebe = NULL;
    reg_data->estadoMae = NULL;
    reg_data->idadeMae = DEFAULT_IDADEMAE;
    reg_data->idNascimento = DEFAULT_IDNASC;
    reg_data->sexoBebe = DEFAULT_SEXOBEBE;
    reg_data->fieldMask = mask;
    return reg_data;
}

/*
    Funcao para desalocar a memoria da struct
    Parametros:
        reg_data_ptr -> endereco da struct que sera desalocada
    Retorno
        nao ha retorno
*/
void virtual_registry_free(VirtualRegistry **reg_data_ptr) {
    #define reg_data (*reg_data_ptr)
    
    if (reg_data_ptr == NULL) {
        DP("ERROR: invalid pointer @virtual_registry_free\n");
        return;
    }

    //Já liberou
    if (reg_data == NULL)
        return;

    free(reg_data->cidadeBebe);
    free(reg_data->cidadeMae);
    free(reg_data->dataNascimento);
    free(reg_data->estadoBebe);
    free(reg_data->estadoMae);
    free(reg_data);
    reg_data = NULL;

    #undef reg_data    
}


/*
    Adiciona um valor a um campo de um VirtualRegistry
    Parametros:
        reg_data -> a struct que sera' modificada do tipo VirtualRegistry
        campo -> string com o nome do campo a ser modificado
        valor -> string com o valor para ser colocado nesse campo
    Retorno:
        nao ha retorno
*/
void virtual_registry_set_field(VirtualRegistry *reg_data, char *field_name, char *field_value) {
    if (reg_data == NULL || field_name == NULL || field_value == NULL) {
        DP("ERROR: Invalid parameters (NULL) in virtual_registry_set_field()\n");
        return;
    }
    
    //Compara o parâmetro com o nome dos campos do registro, atribuindo se for encontrado (dá free em caso de redefinição de string)
    if (!strcmp(field_name, "cidadeBebe")) {
        free(reg_data->cidadeBebe);
        reg_data->cidadeBebe = strdup(field_value);
        return;
    }
    
    if (!strcmp(field_name, "cidadeMae")) {
        free(reg_data->cidadeMae);
        reg_data->cidadeMae = strdup(field_value);
        return;
    }
    
    if (!strcmp(field_name, "dataNascimento")){
        free(reg_data->dataNascimento);
        reg_data->dataNascimento = strdup(field_value);
        return;
    }
    
    if (!strcmp(field_name, "estadoBebe")){
        free(reg_data->estadoBebe);
        reg_data->estadoBebe = strdup(field_value);
        return;
    }
    
    if (!strcmp(field_name, "estadoMae")){
        free(reg_data->estadoMae);
        reg_data->estadoMae = strdup(field_value);
        return;
    }
    
    if (!strcmp(field_name, "idNascimento")){ //Supõe-se que o idNascimento não terá valor inválido
        reg_data->idNascimento = atoi(field_value);
        return;
    }

    if (!strcmp(field_name, "idadeMae")){
        reg_data->idadeMae = (!strcmp(field_value, "") ? -1 : atoi(field_value));
        return;
    }
    
    if (!strcmp(field_name, "sexoBebe")){
        reg_data->sexoBebe = field_value[0];
        return;
    }

    //Se o campo informado não for nenhum dos acima especificados, exibir mensagem de erro
    DP("ERROR: informed field doesn't exist: [%s], at virtual_registry_set_field()\n", field_name);  
} 

//Define o valor encapsulado da mascara de bits do registro
void virtual_registry_set_mask(VirtualRegistry *reg_data, RegistryFieldsMask mask) {
    reg_data->fieldMask = mask;
}

//Retorna o valor encapsulado da mascara de bits do regitstro
RegistryFieldsMask virtual_registry_get_fieldmask(VirtualRegistry *reg_data) {
    return reg_data->fieldMask;
}

/*
    Printa na tela as informacoes da string de acordo com as especificacoes do trabalho
    Parametros:
        reg_data -> VirtualRegistry* que tera' suas informacoes exibidas na tela
    Retorno:
        nao ha retorno
*/
void virtual_registry_print (VirtualRegistry *reg_data) {
    if (reg_data == NULL) {
        DP("Nao ha registro para mostrar em string em virtual_registry_print()\n");
        return;
    }
    
     printf("Nasceu em %s/%s, em %s, um bebê de sexo %s.\n",\
     parse_string_for_print(reg_data->cidadeBebe),          //Faz verificações na string para garantir um char*, e que esteja de acordo com as especificacoes do trabalho
     parse_string_for_print(reg_data->estadoBebe),          //Faz verificações na string para garantir um char*, e que esteja de acordo com as especificacoes do trabalho
     parse_string_for_print(reg_data->dataNascimento),      //Faz verificações na string para garantir um char*, e que esteja de acordo com as especificacoes do trabalho
     parse_sexoBebe_for_print(reg_data->sexoBebe));               //Transforma char -> char*, representando o sexo do bebe, de acordo com as especificacoes do trabalho
}


/*
    Funcao que mostra todas as informacoes do registro na tela
    Parametros:
        reg_data -> VirtualRegistry* que tera' suas informacoes exibidas na tela
    Retorno:
        nao ha retorno
*/
void virtual_registry_print_all_fields (VirtualRegistry *reg_data) {
    if (reg_data == NULL) {
        DP("Nao ha registro para mostrar em string\n");
        return;
    }

    printf("id nasc -> %d\n", reg_data->idNascimento);
    printf("cidade mae -> %s\n", parse_string_for_print(reg_data->cidadeMae));
    printf("estado mae -> %s\n", parse_string_for_print(reg_data->estadoMae));
    printf("cidade bebe -> %s\n", parse_string_for_print(reg_data->cidadeBebe));
    printf("estado bebe -> %s\n", parse_string_for_print(reg_data->estadoBebe));
    printf("data nasc -> %s\n", parse_string_for_print(reg_data->dataNascimento));
    printf("sexo bebe -> %s\n", parse_sexoBebe_for_print(reg_data->sexoBebe));
    printf("idade mae -> %d\n", reg_data->idadeMae);
}

/*
    Funcao que compara dois registros (VirtualRegistry*), levando em conta os campos indicados
    pela mascara (se for um registro cheio, a mascara indica todos os campos, é o caso dos registros lidos do arquivo, por exemplo)
    Parametros:
        reg_data_1 -> registro a ser comparado
        reg_data_2 -> registro a ser comparado
        OBS: a mascara de um registro deve ser o superset da mascara do outro, caso contrário, a operação não faz sentido.
    Retorno:
        bool. true, se os registros forem iguais
              false, se os registros tiverem algum campo com valores diferentes
*/
bool virtual_registry_compare (VirtualRegistry *reg_data_1, VirtualRegistry *reg_data_2) {
    if (reg_data_1 == NULL && reg_data_2 == NULL)
        return true;
    
    if (reg_data_1 == NULL || reg_data_2 == NULL)
        return false;

    //Obtém a mascara que indica quais campos devem ser levados em consideração
    RegistryFieldsMask mask = virtual_registry_get_fieldmask(reg_data_1) & virtual_registry_get_fieldmask(reg_data_2);

    if (mask != virtual_registry_get_fieldmask(reg_data_1) && mask != virtual_registry_get_fieldmask(reg_data_2)) {
        DP("ERROR: the mask of one register must be a superset of the other's mask! @virtual_registry_compare\n");
        return false;
    }

    if ((mask & MASK_CIDADEBEBE) && compare_string_field(reg_data_1->cidadeBebe, reg_data_2->cidadeBebe) == false)
        return false;    

    if ((mask & MASK_CIDADEMAE) && compare_string_field(reg_data_1->cidadeMae, reg_data_2->cidadeMae) == false)
        return false;
        
    if ((mask & MASK_DATANASCIMENTO) && compare_string_field(reg_data_1->dataNascimento, reg_data_2->dataNascimento) == false)
        return false;

    if ((mask & MASK_ESTADOBEBE) && compare_string_field(reg_data_1->estadoBebe, reg_data_2->estadoBebe) == false)
        return false;

    if ((mask & MASK_ESTADOMAE) && compare_string_field(reg_data_1->estadoMae, reg_data_2->estadoMae) == false)
        return false;

    if ((mask & MASK_SEXOBEBE) && reg_data_2->sexoBebe != reg_data_1->sexoBebe)
        return false;

    if ((mask & MASK_IDADEMAE) && reg_data_2->idadeMae != reg_data_1->idadeMae)
        return false;

    if ((mask & MASK_IDNASCIMENTO) && reg_data_2->idNascimento != reg_data_1->idNascimento)
        return false;

    return true;
}


/*
    Funcao para ler de stdin o valor de um determinado campo.
    Caso o campo nao receba um int, usa a funcao scan_quote_string() para le-lo, pois sera um valor entre aspas
    Caso o campo receba um int, usa-se scanf(), pois nao havera aspas para serem tratadas
    Parametros:
        field_name -> o nome do campo que tera seu valor lido
    Retorno:
        char* . O valor do campo que foi lido.
*/
char *virtual_registry_read_value_from_input(char *field_name) {
    if (field_name == NULL) {
        DP("ERROR: invalid parameter in virtual_registry_read_value_from_input()\n");
        return NULL;
    }
    
    char *valor = NULL;
    if (!strcmp("idadeMae", field_name) || !strcmp("idNascimento", field_name))
        scanf(" %ms", &valor);
    else
        scan_quote_string(&valor);

    return valor;
}


/*
    Funcao que le do stdin um numero _m_ de campos e valores para adicionar em um registro, e, depois
    os campos e registros.
    Parametros:
        bool full_register -> indica se o registro é completo ou se é uma busca/atualização. 
            Se for completo, os campos que não forem informados serão escritos com valores padrão no arquivo, 
            se não for completo, os campos que não forem informados serão ignorados.
    Retorno:
        VirtualRegistry* . A struct com valores modificados vindos de stdin
*/
VirtualRegistry *virtual_registry_create_from_input(bool full_register) {
    //Tenta alocar memória
    VirtualRegistry *reg_data = virtual_registry_create_masked(full_register? MASK_ALL: MASK_NONE);
    if (reg_data == NULL) {
        DP("ERROR: Can't allocate memory in virtual_registry_get_registry_fields_from_input()\n");
        return NULL;
    }

    int m;
    char *campo, *valor;
    
    scanf("%d", &m);
    
    for (int i = 0; i < m; i++) {
        scanf(" %ms", &campo);                                  //le o nome do campo

        //Mescla a mascara do campo atual com a que ja estava no registro
        RegistryFieldsMask current_field_mask = registry_mask_from_field_name(campo);
        RegistryFieldsMask current_mask = virtual_registry_get_fieldmask(reg_data);
        virtual_registry_set_mask(reg_data, (current_mask | current_field_mask) );
        
        valor = virtual_registry_read_value_from_input(campo);     //le o valor do campo

        virtual_registry_set_field(reg_data, campo, valor);              //adiciona esse valor a struct em seu campo
        free(campo);
        free(valor);
    }

    return reg_data;
}