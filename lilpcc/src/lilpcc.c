#include "stack.h"
#include "syntax.h"
#include "assembly.h"
#include "../build/parser.tab.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <err.h>

extern Stack *syntax_stack;

extern int yyparse(void);
extern FILE *yyin;

int main(int argc, char *argv[])
{
    ++argv, --argc;
    char *file_name;
    bool ast_only = false;
    if (argc == 1)
    {
        file_name = argv[0];
    }
    else if (argc == 2 && strcmp(argv[0], "--ast-only") == 0)
    {
        file_name = argv[1];
        ast_only = true;
    }
    else
    {
        return 1;
    }

    int result;
    yyin = fopen(file_name, "r");

    if (yyin == NULL)
    {
        printf("Could not open file: '%s'\n", file_name);
        fclose(yyin);
        return 1;
    }

    syntax_stack = stack_new();

    result = yyparse();
    if (result != 0)
    {
        printf("\n");
        stack_free(syntax_stack);
        return 1;
    }

    Syntax *complete_syntax = stack_pop(syntax_stack);
    if (syntax_stack->size > 0)
    {
        warnx("Some elements were not consumed: ");
        while (syntax_stack->size > 0)
        {
            fprintf(stderr, "%s", syntax_type_name(stack_pop(syntax_stack)));
        }
    }

    if (ast_only)
    {
        print_syntax(complete_syntax);
    }
    else
    {
        write_assembly(complete_syntax);
        syntax_free(complete_syntax);
    }

    fclose(yyin);
    return 0;
}
