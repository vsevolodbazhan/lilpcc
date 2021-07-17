#include "symbol_table.h"
#include "context.h"

static const int WORD_SIZE = 4;

void new_scope(Context *ctx)
{
    symbol_table_free(ctx->s_table);
    ctx->s_table = symbol_table_new();

    ctx->stack_offset = 1 * WORD_SIZE;
}

Context *new_context()
{
    Context *ctx = malloc(sizeof(Context));
    ctx->stack_offset = 0;
    ctx->s_table = NULL;
    ctx->label_count = 0;

    return ctx;
}

void context_free(Context *ctx)
{
    symbol_table_free(ctx->s_table);
    free(ctx);
}
