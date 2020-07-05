#pragma once

#include "bool.h"

void static_value_fill_with_garbage(char **value_ptr, int expectedSize);
void registry_prepare_for_write(VirtualRegistry *registry);

bool compare_string_field (char *str1, char *str2);
bool registry_should_update(VirtualRegistry *reg_data, VirtualRegistry *update_reg_data);