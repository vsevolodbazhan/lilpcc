#include "syntax.h"
#include "symbol_table.h"
#include "context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <err.h>

static const int WORD_SIZE = 4;
const int MAX_MNEMONIC_LENGTH = 4;
const char INDENT[5] = "    \0";

static int current_indent = 0;

void decrease_indent()
{
    if (current_indent != 0)
    {
        --current_indent;
    }
}

void increase_indent()
{
    ++current_indent;
}

/* Write instruction INSTR with OPERANDS to OUT.
 *
 * Example:
 * emit_instr(out, "MOV", "%eax, 1");
 */
void emit_instr(FILE *out, int indent, char *instr, char *operands)
{
    while (indent--)
    {
        fprintf(out, INDENT);
    }
    fprintf(out, "%s", instr);

    // Ensure our argument are aligned, regardless of the assembly
    // mnemonic length.
    int argument_offset = MAX_MNEMONIC_LENGTH - strlen(instr) + 4;
    while (argument_offset > 0)
    {
        fprintf(out, " ");
        argument_offset--;
    }

    fprintf(out, "%s\n", operands);
}

/* Write instruction INSTR with formatted operands OPERANDS_FORMAT to
 * OUT.
 *
 * Example:
 * emit_instr_format(out, "MOV", "%%eax, %s", 5);
 */
void emit_instr_format(FILE *out, int indent, char *instr,
                       char *operands_format, ...)
{
    while (indent--)
    {
        fprintf(out, INDENT);
    }
    fprintf(out, "%s", instr);

    // Ensure our argument are aligned, regardless of the assembly
    // mnemonic length.
    int argument_offset = MAX_MNEMONIC_LENGTH - strlen(instr) + 4;
    while (argument_offset > 0)
    {
        fprintf(out, " ");
        argument_offset--;
    }

    va_list argptr;
    va_start(argptr, operands_format);
    vfprintf(out, operands_format, argptr);
    va_end(argptr);

    fputs("\n", out);
}

char *fresh_local_label(char *prefix, Context *ctx)
{
    // We assume we never write more than 6 chars of digits, plus a '.' and '_'.
    size_t buffer_size = strlen(prefix) + 8;
    char *buffer = malloc(buffer_size);

    snprintf(buffer, buffer_size, ".%s_%d", prefix, ctx->label_count);
    ctx->label_count++;

    return buffer;
}

void emit_label(FILE *out, int indent, char *label)
{
    while (indent--)
    {
        fprintf(out, INDENT);
    }
    fprintf(out, "%s:\n", label);
    increase_indent();
}

void emit_call(FILE *out, int indent, char *name)
{
    emit_instr_format(out, indent, "jal", "%s", name);
}

void emit_function_declaration(FILE *out, int indent, char *name)
{
    emit_label(out, indent, name);
}

void emit_return(FILE *out, int indent)
{
    emit_instr_format(out, indent, "jr", "$ra");
}

void emit_function_epilogue(FILE *out)
{
    fprintf(out, "\n");
}

void write_syntax(FILE *out, Syntax *syntax, Context *ctx)
{
    if (syntax->type == UNARY_OPERATOR)
    {
        UnaryExpression *unary_syntax = syntax->unary_expression;

        write_syntax(out, unary_syntax->expression, ctx);
        emit_instr(out, current_indent, "move", "$t1, $t0");
        emit_instr_format(out, current_indent, "li", "$t0, %d", "0xffffffff");
        emit_instr_format(out, current_indent, "xor", "$t0, $t0, $t1");
    }
    else if (syntax->type == IMMEDIATE)
    {
        emit_instr_format(out, current_indent, "li", "$t0, %d",
                          syntax->immediate->value);
    }
    else if (syntax->type == VARIABLE)
    {
        emit_instr_format(
            out, current_indent, "lw", "$t0, %d($sp)",
            symbol_table_get_offset(ctx->s_table, syntax->variable->var_name));
    }
    else if (syntax->type == BINARY_OPERATOR)
    {
        BinaryExpression *binary_syntax = syntax->binary_expression;
        ctx->stack_offset += WORD_SIZE;

        write_syntax(out, binary_syntax->left, ctx);
        emit_instr(out, current_indent, "move", "$t1, $t0");
        write_syntax(out, binary_syntax->right, ctx);

        if (binary_syntax->binary_type == MULTIPLICATION)
        {
            emit_instr_format(out, current_indent,
                              "mul", "$t0, $t1, $t0");
        }
        if (binary_syntax->binary_type == MULTIPLICATION)
        {
            emit_instr_format(out, current_indent,
                              "div", "$t0, $t1, $t0");
        }
        else if (binary_syntax->binary_type == ADDITION)
        {
            emit_instr_format(out, current_indent, "addu",
                              "$t0, $t1, $t0");
        }
        else if (binary_syntax->binary_type == SUBTRACTION)
        {
            emit_instr_format(out, current_indent, "subu",
                              "$t0, $t1, $t0");
        }
        else if (binary_syntax->binary_type == LESS_THAN)
        {
            emit_instr_format(out, current_indent, "slt", "$t0, $t1, $t0");
        }
        else if (binary_syntax->binary_type == LESS_THAN_OR_EQUAL)
        {
            char *true_label = fresh_local_label("if_less_or_equal", ctx);
            char *false_label = fresh_local_label("continue", ctx);

            emit_instr_format(out, current_indent, "ble", "$t1, $t0, %s",
                              true_label);
            emit_instr_format(out, current_indent, "li", "$t0, 0");
            emit_instr_format(out, current_indent, "j", "%s",
                              false_label);

            emit_label(out, current_indent, true_label);
            emit_instr_format(out, current_indent, "li", "$t0, 1");

            decrease_indent();
            emit_label(out, current_indent, false_label);
            decrease_indent();
            emit_instr_format(out, current_indent, "ori", "$t0, $t0, 0");
        }
        else if (binary_syntax->binary_type == GREATER_THAN)
        {
            char *true_label = fresh_local_label("if_greater", ctx);
            char *false_label = fresh_local_label("continue", ctx);

            emit_instr_format(out, current_indent, "bgt", "$t1, $t0, %s",
                              true_label);
            emit_instr_format(out, current_indent, "li", "$t0, 0");
            emit_instr_format(out, current_indent, "j", "%s",
                              false_label);

            emit_label(out, current_indent, true_label);
            emit_instr_format(out, current_indent, "li", "$t0, 1");

            decrease_indent();
            emit_label(out, current_indent, false_label);
            decrease_indent();
            emit_instr_format(out, current_indent, "ori", "$t0, $t0, 0");
        }
        else if (binary_syntax->binary_type == GREATER_THAN_OR_EQUAL)
        {
            char *true_label = fresh_local_label("if_greater_or_equal", ctx);
            char *false_label = fresh_local_label("continue", ctx);

            emit_instr_format(out, current_indent, "bge", "$t1, $t0, %s",
                              true_label);
            emit_instr_format(out, current_indent, "li", "$t0, 0");
            emit_instr_format(out, current_indent, "j", "%s",
                              false_label);

            emit_label(out, current_indent, true_label);
            emit_instr_format(out, current_indent, "li", "$t0, 1");

            decrease_indent();
            emit_label(out, current_indent, false_label);
            decrease_indent();
            emit_instr_format(out, current_indent, "ori", "$t0, $t0, 0");
        }
    }
    else if (syntax->type == ASSIGNMENT)
    {
        write_syntax(out, syntax->assignment->expression, ctx);
        emit_instr_format(
            out, current_indent, "sw", "$t0, %d($sp)",
            symbol_table_get_offset(ctx->s_table, syntax->variable->var_name));
    }
    else if (syntax->type == RETURN_STATEMENT)
    {
        ReturnStatement *return_statement = syntax->return_statement;
        write_syntax(out, return_statement->expression, ctx);
        emit_return(out, current_indent);
    }
    else if (syntax->type == FUNCTION_CALL)
    {
        emit_call(out, current_indent, syntax->function_call->function_name);
    }
    else if (syntax->type == IF_STATEMENT)
    {
        char *true_label = fresh_local_label("if_true", ctx);
        char *false_label = fresh_local_label("continue", ctx);

        IfStatement *if_statement = syntax->if_statement;
        write_syntax(out, if_statement->condition, ctx);

        emit_instr_format(out, current_indent, "bne", "$t0, $0, %s",
                          true_label);
        emit_instr_format(out, current_indent, "j", "%s",
                          false_label);

        emit_label(out, current_indent, true_label);
        write_syntax(out, if_statement->then, ctx);

        emit_label(out, current_indent, false_label);
        decrease_indent();
    }
    else if (syntax->type == WHILE_SYNTAX)
    {
        char *true_label = fresh_local_label("while_true", ctx);
        char *false_label = fresh_local_label("continue", ctx);

        WhileStatement *while_statement = syntax->while_statement;
        write_syntax(out, while_statement->condition, ctx);

        emit_instr_format(out, current_indent, "bne", "$t0, $0, %s",
                          true_label);
        emit_instr_format(out, current_indent, "j", "%s",
                          false_label);

        emit_label(out, current_indent, true_label);
        write_syntax(out, while_statement->body, ctx);

        write_syntax(out, while_statement->condition, ctx);
        emit_instr_format(out, current_indent, "bne", "$t0, $0, %s",
                          true_label);

        emit_label(out, current_indent, false_label);
        decrease_indent();
    }
    else if (syntax->type == DEFINE_VAR)
    {
        DefineVarStatement *define_var_statement = syntax->define_var_statement;
        int stack_offset = ctx->stack_offset;

        symbol_table_set_offset(ctx->s_table, define_var_statement->var_name,
                                stack_offset);
        emit_instr_format(out, current_indent, "lw", "$t0, %d($sp)",
                          stack_offset);

        ctx->stack_offset += WORD_SIZE;
        write_syntax(out, define_var_statement->init_value, ctx);
        emit_instr_format(out, current_indent, "sw", "$t0, %d($sp)",
                          stack_offset);
    }
    else if (syntax->type == BLOCK)
    {
        List *statements = syntax->block->statements;
        for (int i = 0; i < list_length(statements); i++)
        {
            write_syntax(out, list_get(statements, i), ctx);
        }
        decrease_indent();
    }
    else if (syntax->type == FUNCTION)
    {
        new_scope(ctx);
        emit_function_declaration(out, current_indent, syntax->function->name);
        write_syntax(out, syntax->function->root_block, ctx);
        emit_function_epilogue(out);
    }
    else if (syntax->type == TOP_LEVEL)
    {
        List *declarations = syntax->top_level->declarations;
        for (int i = 0; i < list_length(declarations); i++)
        {
            write_syntax(out, list_get(declarations, i), ctx);
        }
    }
    else
    {
        warnx("Unknown syntax %s", syntax_type_name(syntax));
        assert(false);
    }
}

void write_assembly(Syntax *syntax)
{
    FILE *out = fopen("out.asm", "wb");

    Context *ctx = new_context();
    emit_instr(out, current_indent, "j", "main\n");
    write_syntax(out, syntax, ctx);
    context_free(ctx);

    fclose(out);
}
