#ifndef __REGISTRY_MASK__H__
#define __REGISTRY_MASK__H__

/**
 *  Possíveis mascaras de campo que definem quais campos serão usados em registros
 *  do tipo mascarado.
 *  Uso proposto:
 *      mascara_atual |=  outra_mascara; -> adiciona mascara
 *      mascara_atual &= ~outra_mascara; -> remove mascara
 *      if (mascara_atual & (outra_mascara1 | outra_mascara2) ){} -> checa se possui, ou a mascara 1, ou a mascara 2
 */
enum _virtual_registry_mask {
    MASK_NONE = 0,
    MASK_CIDADEMAE = 1,
    MASK_CIDADEBEBE = 2,
    MASK_IDNASCIMENTO = 4,
    MASK_IDADEMAE = 8,
    MASK_DATANASCIMENTO = 16,
    MASK_SEXOBEBE = 32,
    MASK_ESTADOMAE = 64,
    MASK_ESTADOBEBE = 128,
    MASK_ALL = 255
};

typedef enum _virtual_registry_mask RegistryFieldsMask;

RegistryFieldsMask registry_mask_from_field_name(char *field_name);

#endif  //!__REGISTRY_MANAGER__H__