#include "registry_mask.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"


/**
 *  Converte o nome do campo em string para uma mascara de bits.
 *  Utilizado na leitura de filtros de busca e atualizações.
 *  Parâmetros:
 *      char *field_name -> nome do campo
 *  Retorno:
 *      RegistryFieldsMask -> máscara pura referente ao campo informado
 */
RegistryFieldsMask registry_mask_from_field_name(char *field_name) {
    if (field_name == NULL) {
        DP("ERROR: Invalid parameters (NULL) in registry_mask_from_field_name()\n");
        return MASK_NONE;
    }
    
    //Compara o parâmetro com o nome dos campos do registro, retornando a mascara adequada
    if (!strcmp(field_name, "cidadeBebe"))      return MASK_CIDADEBEBE;
    if (!strcmp(field_name, "cidadeMae"))       return MASK_CIDADEMAE;
    if (!strcmp(field_name, "dataNascimento"))  return MASK_DATANASCIMENTO;
    if (!strcmp(field_name, "estadoBebe"))      return MASK_ESTADOBEBE;
    if (!strcmp(field_name, "estadoMae"))       return MASK_ESTADOMAE;
    if (!strcmp(field_name, "idNascimento"))    return MASK_IDNASCIMENTO;
    if (!strcmp(field_name, "idadeMae"))        return MASK_IDADEMAE;
    if (!strcmp(field_name, "sexoBebe"))        return MASK_SEXOBEBE;

    //Se o campo informado não for nenhum dos acima especificados, exibir mensagem de erros
    DP("ERROR: informed field doesn't exist: [%s], at registry_mask_from_field_name()\n", field_name); 
    return MASK_NONE;
}
