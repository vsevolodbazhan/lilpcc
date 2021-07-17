#include "symbol_table.h"

#ifndef LILPCC_CTX_HEADER
#define LILPCC_CTX_HEADER

typedef struct Context
{
    int stack_offset;
    SymbolTable *s_table;
    int label_count;
} Context;

void new_scope(Context *ctx);

Context *new_context();

void context_free(Context *ctx);

#endif
