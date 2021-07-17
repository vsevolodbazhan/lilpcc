#include "symbol_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

SymbolTable *symbol_table_new()
{
    SymbolTable *s_table = malloc(sizeof(SymbolTable));
    s_table->size = 0;
    s_table->items = NULL;

    return s_table;
}

void symbol_table_set_offset(SymbolTable *s_table, char *var_name, int offset)
{
    s_table->size++;
    s_table->items = realloc(s_table->items, s_table->size * sizeof(VarWithOffset));

    VarWithOffset *vwo = &s_table->items[s_table->size - 1];
    vwo->var_name = var_name;
    vwo->offset = offset;
}

/* Return the offset of variable VAR_NAME. */
int symbol_table_get_offset(SymbolTable *s_table, char *var_name)
{
    VarWithOffset vwo;
    for (size_t i = 0; i < s_table->size; i++)
    {
        vwo = s_table->items[i];

        if (strcmp(vwo.var_name, var_name) == 0)
        {
            return vwo.offset;
        }
    }

    warnx("Could not find %s in s_tableironment", var_name);
    return -1;
}

void symbol_table_free(SymbolTable *s_table)
{
    if (s_table != NULL)
    {
        free(s_table->items);
        free(s_table);
    }
}
