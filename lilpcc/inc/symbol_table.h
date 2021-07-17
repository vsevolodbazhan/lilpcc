#include <stdlib.h>

#ifndef LILPCC_SYMBOL_TABLE_HEADER
#define LILPCC_SYMBOL_TABLE_HEADER

typedef struct VarWithOffset
{
    char *var_name;
    int offset;
} VarWithOffset;

typedef struct SymbolTable
{
    size_t size;
    VarWithOffset *items;
} SymbolTable;

SymbolTable *symbol_table_new();

void symbol_table_set_offset(SymbolTable *s_table, char *var_name, int offset);

int symbol_table_get_offset(SymbolTable *s_table, char *var_name);

void symbol_table_free(SymbolTable *s_table);

#endif
