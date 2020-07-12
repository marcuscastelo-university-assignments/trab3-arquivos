#ifndef __BINARY_REGISTRY__H__
#define __BINARY_REGISTRY__H__

#include "registry.h"

bool binary_update_registry(FILE *file, VirtualRegistry *updated_reg);
bool binary_write_registry(FILE *file, VirtualRegistry *reg_data);
VirtualRegistry *binary_read_registry(FILE *file);

#endif  //!__BINARY_REGISTRY__H__