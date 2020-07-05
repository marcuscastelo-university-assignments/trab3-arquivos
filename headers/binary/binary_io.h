#ifndef BINARYIO_H
#define BINARYIO_H

#include <stdio.h>

#define REG_VARIABLE_FIELDS_TOTAL_SIZE 105

void binary_write_int(FILE *file, int num);
int binary_read_int(FILE *file);

void binary_write_char(FILE *file, char param);
char binary_read_char(FILE *file);

void binary_write_string(FILE *file, char* param, int size);
char *binary_read_string(FILE *file, int size);

int calculate_variable_fields_garbage(int strings_size_sum);
char *generate_garbage(int strings_size_sum);

#endif