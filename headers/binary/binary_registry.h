#pragma once

#include "registry.h"

void binary_update_registry(FILE *file, VirtualRegistry *updated_reg);
void binary_write_registry(FILE *file, VirtualRegistry *reg_data);
VirtualRegistry *binary_read_registry(FILE *file);
