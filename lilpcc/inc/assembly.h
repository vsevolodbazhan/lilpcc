#include "context.h"
#include "symbol_table.h"
#include "syntax.h"

#include <stdio.h>

#ifndef LILPCC_ASSEMBLY_HEADER
#define LILPCC_ASSEMBLY_HEADER

void decrease_indent();

void increase_indent();

void emit_insn(FILE *out, char *insn);

void emit_instr_format(FILE *out, int indent, char *instr,
                       char *operands_format, ...);

char *fresh_local_label(char *prefix, Context *ctx);

void emit_label(FILE *out, int indent, char *label);

void emit_call(FILE *out, int indent, char *name);

void emit_function_declaration(FILE *out, int indent, char *name);

void emit_return(FILE *out, int indent);

void emit_function_epilogue(FILE *out);

void write_syntax(FILE *out, Syntax *syntax);

void write_assembly(Syntax *syntax);

#endif
