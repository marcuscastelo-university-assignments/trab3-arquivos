#pragma once

#include <stdio.h>

#include "bool.h"
#include "registry.h"

#define INCREASE -1
#define DECREASE -2

#define FRONT 1
#define BACK -1

typedef struct _registry_manager RegistryManager;

RegistryManager *registry_manager_create(FILE *bin_file, RegistryHeader *header);
void registry_manager_delete(RegistryManager **manager_ptr);

void registry_manager_seek(RegistryManager *manager, int RRN);
void registry_manager_seek_first(RegistryManager *manager);
void registry_manager_seek_end(RegistryManager *manager);

VirtualRegistry *registry_manager_read_current(RegistryManager *manager);
VirtualRegistry *registry_manager_read_at(RegistryManager *manager, int RRN);

void registry_manager_write_current(RegistryManager *manager, VirtualRegistry *reg_data);
void registry_manager_write_at(RegistryManager *manager, int RRN, VirtualRegistry *reg_data);

void registry_manager_write_current_field_by_num(RegistryManager *manager, VirtualRegistry *reg_data, int field_num);

void registry_manager_update_current(RegistryManager *manager, VirtualRegistry *reg_data);
void registry_manager_update_at(RegistryManager *manager, int RRN, VirtualRegistry *reg_data);

void registry_manager_delete_current (RegistryManager *manager);
void registry_manager_delete_at (RegistryManager *manager, int RRN);
bool registry_manager_is_current_deleted (RegistryManager *manager);

void registry_manager_jump_registry (RegistryManager *manager, int direction);


