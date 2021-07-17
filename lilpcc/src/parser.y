%{
#include "../inc/syntax.h"
#include "../inc/stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define YYSTYPE char*

int yyparse(void);
int yylex();

void yyerror(const char * message)
{
    fprintf(stderr, "ERROR: %s\n", message);
}

int yywrap()
{
    return 1;
}

extern FILE *yyin;

Stack *syntax_stack;
%}

%token OP_BRACE CL_BRACE OP_PAREN CL_PAREN
%token ID NUMBER IF WHILE RETURN INT
%token ASSIGN SEMICOLON COMMA
%token XOR PLUS MINUS MULT DIV
%token LESS GREATER LESS_OR_EQUAL GREATER_OR_EQUAL EQUAL

%left MULT DIV
%left PLUS MINUS
%left LESS LESS_OR_EQUAL GREATER GREATER_OR_EQUAL
%left EQUAL
%left XOR
%left ASSIGN
%left COMMA

%%
program :
    function program
    {
        Syntax *top_level_syntax;
        if (stack_empty(syntax_stack)) {
            top_level_syntax = top_level_new();
        } else if (((Syntax *)stack_peek(syntax_stack))->type != TOP_LEVEL) {
            top_level_syntax = top_level_new();
        } else {
            top_level_syntax = stack_pop(syntax_stack);
        }

        list_push(top_level_syntax->top_level->declarations,
                    stack_pop(syntax_stack));
        stack_push(syntax_stack, top_level_syntax);
    }
    |
    {
        stack_push(syntax_stack, top_level_new());
    }
    ;

function :
    INT ID OP_PAREN parameter_list CL_PAREN OP_BRACE block CL_BRACE
    {
        Syntax *current_syntax = stack_pop(syntax_stack);
        stack_push(syntax_stack, function_new((char*)$2, current_syntax));
    }
    ;

parameter_list :
    non_empty_parameter_list
    |
    ;

non_empty_parameter_list :
    INT ID COMMA parameter_list
    | INT ID
    ;

block :
    statement block
    {
        Syntax *block_syntax;
        if (stack_empty(syntax_stack)) {
            block_syntax = block_new(list_new());
        } else if (((Syntax *)stack_peek(syntax_stack))->type != BLOCK) {
            block_syntax = block_new(list_new());
        } else {
            block_syntax = stack_pop(syntax_stack);
        }

        list_push(block_syntax->block->statements, stack_pop(syntax_stack));
        stack_push(syntax_stack, block_syntax);
    }
    |
    {
        stack_push(syntax_stack, block_new(list_new()));
    }
    ;

argument_list :
    non_empty_argument_list
    |
    {
        stack_push(syntax_stack, function_arguments_new());
    }
    ;

non_empty_argument_list :
    expression COMMA non_empty_argument_list
    {
        Syntax *arguments_syntax;
        if (stack_empty(syntax_stack)) {
            assert(false);
        } else if (((Syntax *)stack_peek(syntax_stack))->type != FUNCTION_ARGUMENTS) {
            arguments_syntax = function_arguments_new();
        } else {
            arguments_syntax = stack_pop(syntax_stack);
        }

        list_push(arguments_syntax->function_arguments->arguments, stack_pop(syntax_stack));
        stack_push(syntax_stack, arguments_syntax);
    }
    | expression
    {
        if (stack_empty(syntax_stack)) {
            assert(false);
        }

        Syntax *arguments_syntax = function_arguments_new();
        list_push(arguments_syntax->function_arguments->arguments, stack_pop(syntax_stack));

        stack_push(syntax_stack, arguments_syntax);
    }
    ;

statement :
    RETURN expression SEMICOLON
    {
        Syntax *current_syntax = stack_pop(syntax_stack);
        stack_push(syntax_stack, return_statement_new(current_syntax));
    }
    | IF OP_PAREN expression CL_PAREN OP_BRACE block CL_BRACE
    {
        Syntax *then = stack_pop(syntax_stack);
        Syntax *condition = stack_pop(syntax_stack);
        stack_push(syntax_stack, if_new(condition, then));
    }
    | WHILE OP_PAREN expression CL_PAREN OP_BRACE block CL_BRACE
    {
        Syntax *body = stack_pop(syntax_stack);
        Syntax *condition = stack_pop(syntax_stack);
        stack_push(syntax_stack, while_new(condition, body));
    }
    | INT ID ASSIGN expression SEMICOLON
    {
        Syntax *init_value = stack_pop(syntax_stack);
        stack_push(syntax_stack, define_var_new((char*)$2, init_value));
    }
    | expression SEMICOLON {}
    ;

expression :
    NUMBER
    {
        stack_push(syntax_stack, immediate_new(atoi((char*)$1)));
        free($1);
    }
    | ID
    {
        stack_push(syntax_stack, variable_new((char*)$1));
    }
    | ID ASSIGN expression
    {
        Syntax *expression = stack_pop(syntax_stack);
        stack_push(syntax_stack, assignment_new((char*)$1, expression));
    }
    | XOR expression
    {
        Syntax *current_syntax = stack_pop(syntax_stack);
        stack_push(syntax_stack, bitwise_negation_new(current_syntax));
    }
    | expression PLUS expression
    {
        Syntax *right = stack_pop(syntax_stack);
        Syntax *left = stack_pop(syntax_stack);
        stack_push(syntax_stack, addition_new(left, right));
    }
    | expression MINUS expression
    {
        Syntax *right = stack_pop(syntax_stack);
        Syntax *left = stack_pop(syntax_stack);
        stack_push(syntax_stack, subtraction_new(left, right));
    }
    | expression MULT expression
    {
        Syntax *right = stack_pop(syntax_stack);
        Syntax *left = stack_pop(syntax_stack);
        stack_push(syntax_stack, multiplication_new(left, right));
    }
    | expression DIV expression
    {
        Syntax *right = stack_pop(syntax_stack);
        Syntax *left = stack_pop(syntax_stack);
        stack_push(syntax_stack, division_new(left, right));
    }
    | expression LESS expression
    {
        Syntax *right = stack_pop(syntax_stack);
        Syntax *left = stack_pop(syntax_stack);
        stack_push(syntax_stack, less_than_new(left, right));
    }
    | expression LESS_OR_EQUAL expression
    {
        Syntax *right = stack_pop(syntax_stack);
        Syntax *left = stack_pop(syntax_stack);
        stack_push(syntax_stack, less_or_equal_new(left, right));
    }
    | expression GREATER expression
    {
        Syntax *right = stack_pop(syntax_stack);
        Syntax *left = stack_pop(syntax_stack);
        stack_push(syntax_stack, greater_than_new(left, right));
    }
    | expression GREATER_OR_EQUAL expression
    {
        Syntax *right = stack_pop(syntax_stack);
        Syntax *left = stack_pop(syntax_stack);
        stack_push(syntax_stack, greater_or_equal_new(left, right));
    }
    | ID OP_PAREN argument_list CL_PAREN
    {
        Syntax *arguments = stack_pop(syntax_stack);
        stack_push(syntax_stack, function_call_new((char*)$1, arguments));
    }
    ;
%%
