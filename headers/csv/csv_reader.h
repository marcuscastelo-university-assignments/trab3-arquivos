#ifndef __CSV_READER__H__
#define __CSV_READER__H__

#include "registry.h"
#include "registry_array.h"

typedef struct csv_reader CsvReader;

VirtualRegistryArray *csv_read_all_lines(const char *file_name);

#endif  //!__CSV_READER__H__