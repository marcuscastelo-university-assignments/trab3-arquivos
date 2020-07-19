#ifndef __CSV_READER__H__
#define __CSV_READER__H__

#include "registry.h"
#include "registry_array.h"
#include "open_mode.h"

typedef struct csv_reader_ CsvReader;

CsvReader *csv_reader_create(void);
OPEN_RESULT csv_reader_open(CsvReader *reader, char *csv_filename);
VirtualRegistry *csv_reader_readline(CsvReader *reader);
void csv_reader_close(CsvReader *reader);
void csv_reader_free(CsvReader **reader_ptr);

#endif  //!__CSV_READER__H__