/*
 *   Derek Aguirre
 *   Grace Auten
 *   Leah Howells
 *   Brendan Smith
 *
 *   Assignment 3
 *   COP 3402; Section 2
 */
#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lexeme *tokens;
int token_index = 0;
symbol *table;
int table_index = 0;
instruction *code;
int code_index = 0;

int error = 0;
int level;

void program();
void block();

/*  Declaration Functions  */
void declarations();
void const_func();
void var();
void proc();

/*  Statement Functions  */
void statement();
void ident();
void call();
void begin();
void if_func();
void while_func();
void read();
void write();
void def();
void return_func();

/*  Condition  */
void condition();

/*  Expression  */
void expression();

/*  Term  */
void term();

/*  Factor  */
void factor();
void factor_ident();
void factor_left_parenthesis();

int const_follow_statement(token_type t);
void emit(int op, int l, int m);
void add_symbol(int kind, char name[], int value, int level, int address);
void mark();
int multiple_declaration_check(char name[]);
int find_symbol(char name[], int kind);

void print_parser_error(int error_code, int case_code);
void print_assembly_code();
void print_symbol_table();

instruction *parse(int code_flag, int table_flag, lexeme *list)
{
    tokens = list;
    table = calloc(ARRAY_SIZE, sizeof(symbol));
    code = calloc(ARRAY_SIZE, sizeof(instruction));

    program();

    if (!error && code_flag)
        print_assembly_code();

    if (!error && table_flag)
        print_symbol_table();

    if (!error)
        return code;

    return NULL;
}

void program()
{
    level = -1;

    block();

    while (tokens[token_index].type != 0)
        ++token_index;

    if (tokens[token_index - 1].type != 17)
    {
        print_parser_error(1, 0); // always non-stopping
        error = 1;
    }

    int i = 0;
    while (code[i].op != 0)
    {
        instruction inst = code[i];

        if (inst.op == 5 && inst.m != -1 && table[inst.m].address != -1)
            code[i].m = table[inst.m].address;

        else if (inst.op == 5 && inst.m != -1)
        {
            print_parser_error(21, 0); // always non-stopping
            error = 1;
        }

        ++i;
    }

    if (code[code_index - 1].op != 10 || code[code_index - 1].m != 3)
        emit(10, 0, 3);
}

void block()
{
    if (error == -1)
        return;

    ++level;

    declarations();
    statement();
    mark();

    --level;
}

/*  Declaration Functions  */
void declarations()
{
    if (error == -1)
        return;

    int m = 0;
    token_type t = tokens[token_index++].type;

    /*  Checks keyword  (const, var, proc)  */
    while (t == 3 || t == 4 || t == 5)
    {
        switch (t)
        {
        case 3:
            const_func();
            break;
        case 4:
            var(m);
            ++m;
            break;
        default:
            proc();
        }

        t = tokens[token_index++].type;
    }
    --token_index;

    emit(7, 0, m + 3);
}

void const_func()
{
    int symbol_value = 0, minus = 0;
    char name[12];

    if (error == -1)
        return;

    if (tokens[token_index].type == 1)
    {
        if (multiple_declaration_check(tokens[token_index].identifier_name) == -1)
            strcpy(name, tokens[token_index].identifier_name);

        else
        {
            print_parser_error(3, 0); // always non-stopping
            error = 1;
        }

        ++token_index;
    }

    else
    {
        print_parser_error(2, 1); // sometimes non-stopping

        if (tokens[token_index].type == 18)
        {
            strcpy(name, "null");
            error = 1;
        }
        
        else
        {
            error = -1;
            return;
        }
    }

    /*  Shorthand Declaration Check  */
    if (tokens[token_index].type == 18)
    {
        ++token_index;
    }
    else
    {
        print_parser_error(4, 1); // sometimes non-stopping

        /* If either of the next 2 tokens are numbers, then this is not a stopping
         * error. If this is not the case, then it is a stopping error. */

        if (tokens[token_index].type == 2 || tokens[token_index].type == 19)
            error = 1;

        else
        {
            error = -1;
            return;
        }
    }

    /*  Checks for minus, no error if its not there  */
    if (tokens[token_index].type == 19)
    {
        ++token_index;
        minus = 1;
    }

    if (tokens[token_index].type == 2)
    {
        symbol_value = tokens[token_index++].number_value;

        if (minus)
        {
            symbol_value *= -1;
        }
    }
    else
    {
        print_parser_error(5, 0); // sometimes non-stopping

        /*  Has semicolon  */
        if (tokens[token_index].type == 20)
        {
            symbol_value = 0;
            error = 1;
        }

        else
        {
            error = -1;
            return;
        }
    }

    if (tokens[token_index].type == 20)
    {
        ++token_index;
    }
    else
    {
        print_parser_error(6, 1); // sometimes non-stopping

        if (const_follow_statement(tokens[token_index].type))
            error = 1;

        else
        {
            error = -1;
            return;
        }
    }

    add_symbol(1, name, symbol_value, level, 0);
}

void var(int m)
{
    char name[12];

    if (error == -1)
        return;

    if (tokens[token_index].type == 1)
    {
        if (multiple_declaration_check(tokens[token_index].identifier_name) == -1)
        {
            strcpy(name, tokens[token_index].identifier_name);
        }
        else
        {
            print_parser_error(3, 0); // always non-stopping
            error = 1;
        }

        ++token_index;
    }

    else
    {
        print_parser_error(2, 2); // sometimes non-stopping

        if (tokens[token_index].type == 20)
        {
            strcpy(name, "null");
            error = 1;
        }
        else
        {
            error = -1;
            return;
        }
    }

    if (tokens[token_index].type == 20)
    {
        ++token_index;
    }
    else
    {
        print_parser_error(6, 2); // sometimes non-stopping

        if (const_follow_statement(tokens[token_index].type))
            error = 1;

        else
        {
            error = -1;
            return;
        }
    }

    add_symbol(2, name, 0, level, m + 3);
}

void proc()
{
    char name[12];

    if (error == -1)
        return;

    if (tokens[token_index].type == 1)
    {
        if (multiple_declaration_check(tokens[token_index].identifier_name) == -1)
        {
            strcpy(name, tokens[token_index].identifier_name);
        }
        else
        {
            print_parser_error(3, 0); // always non-stopping
            error = 1;
        }

        ++token_index;
    }

    else
    {
        print_parser_error(2, 3); // sometimes non-stopping

        if (tokens[token_index].type == 20)
        {
            strcpy(name, "null");
            error = 1;
        }
        else
        {
            error = -1;
            return;
        }
    }

    if (tokens[token_index].type == 20)
    {
        ++token_index;
    }
    else
    {
        print_parser_error(6, 3); // sometimes non-stopping

        if (const_follow_statement(tokens[token_index].type))
            error = 1;

        else
        {
            error = -1;
            return;
        }
    }

    add_symbol(3, name, 0, level, -1);
}

/*  Statement Functions  */
void statement()
{
    if (error == -1)
        return;

    switch (tokens[token_index].type)
    {
    case 1:
        ident();
        break;
    case 6:
        ++token_index;
        call();
        break;
    case 7:
        begin();
        break;
    case 9:
        ++token_index;
        if_func();
        break;
    case 11:
        ++token_index;
        while_func();
        break;
    case 13:
        ++token_index;
        read();
        break;
    case 14:
        ++token_index;
        write();
        break;
    case 15:
        ++token_index;
        return_func();
        break;
    case 16:
        ++token_index;
        def();
        break;
    default:
        return;
    }
}

void ident()
{
    if (error == -1)
        return;

    /*  Variables  */
    int emit_level = 0, m = 0;
    token_type t = tokens[token_index].type;
    char name[12];
    strcpy(name, tokens[token_index].identifier_name);

    int index = find_symbol(name, 2);

    /*  If all kinds == -1, no identifier by that name  */
    if (find_symbol(name, 1) == -1 && index == -1 && find_symbol(name, 3) == -1)
    {
        print_parser_error(8, 1); // always non-stopping

        emit_level = -1;
        m = -1;
        error = 1;
    }

    /*  Identifier by the name, but not of the right type  */
    else if (index == -1)
    {
        print_parser_error(7, 0); // always non-stopping

        emit_level = -1;
        m = -1;
        error = 1;
    }

    /*  Identifier var found  */
    else
    {
        symbol s = table[index];
        emit_level = level - s.level;
        m = s.address;
    }

    ++token_index;

    t = tokens[token_index].type; // Placeholder var

    /*  Checks for assignment statement and handles error accordingly  */
    switch (t)
    {
    case 18:
        ++token_index;
        break;
    case 1:
    case 2:
    case 32:
        print_parser_error(4, 2); // sometimes non-stopping
        error = 1;
        break;
    default:
        print_parser_error(4, 2); // sometimes non-stopping
        error = -1;
        return;
    }

    expression();

    emit(4, emit_level, m);
}

void call()
{
    if (error == -1)
        return;

    /*  Vars  */
    int emit_level = 0, m = 0, bool = 1;
    token_type t = tokens[token_index].type;
    char name[12];
    strcpy(name, tokens[token_index].identifier_name);

    /*  If not an Identifier, check follow set for if error is stopping  */
    if (t != 1)
    {
        print_parser_error(2, 4); // sometimes non-stopping

        switch (t)
        {
        case 17:
        case 20:
        case 22:
        case 8:
            --token_index;
            emit_level = -1;
            m = -1;
            bool = 0;
            error = 1;
            break;
        default:
            error = -1;
            return;
        }
    }

    int index = find_symbol(name, 3);
    symbol s = table[index];

    /*  Same as in ident  */
    // printf("%d\n", tokens[token_index].type);
    if (bool &&find_symbol(name, 1) == -1 && find_symbol(name, 2) == -1 && index == -1)
    {
        print_parser_error(8, 2); // always non-stopping

        emit_level = -1;
        m = -1;
        error = 1;
    }

    else if (bool &&index == -1)
    {
        print_parser_error(9, 0); // always non-stopping

        emit_level = -1;
        m = -1;
        error = 1;
    }

    else if (bool)
    {
        emit_level = level - s.level;
        m = index;
    }

    ++token_index;

    emit(5, emit_level, m);
}

void begin()
{
    if (error == -1)
        return;

    /*  Loops while there's a semicolon  */
    do
    {
        ++token_index;
        statement();

        if (error == -1)
            return;

    } while (tokens[token_index].type == 20);

    token_type t = tokens[token_index].type;
    /*  If keyword_end: continue; If not, determine the proper follow set
        and handle the errors accordingly  */
    switch (t)
    {
    case 8:
        ++token_index;
        break;
    case 1:
    case 6:
    case 7:
    case 9:
    case 11:
    case 13:
    case 14:
    case 15:
    case 16:
        print_parser_error(6, 4); // always stopping
        error = -1;
        return;
    case 17:
    case 20:
    case 22:
        print_parser_error(10, 0); // sometimes non-stopping
        error = 1;
        break;
    default:
        print_parser_error(10, 0); // sometimes non-stopping
        error = -1;
        return;
    }
}

void if_func()
{
    if (error == -1)
        return;

    condition();

    int temp_code_index = code_index;
    emit(9, 0, 0);

    /*  Follow set if "then" isn't after if  */
    switch (tokens[token_index].type)
    {
    case 10:
        token_index++;
        break;
    case 1:
    case 6:
    case 7:
    case 8:
    case 9:
    case 11:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 20:
    case 22:
        print_parser_error(11, 0); // sometimes non-stopping
        error = 1;
        break;
    default:
        print_parser_error(11, 0); // sometimes non-stopping
        error = -1;
        return;
    }

    statement();

    code[temp_code_index].m = code_index;
}

void while_func()
{
    if (error == -1)
        return;

    int jmp_index = code_index;
    condition();

    int jpc_index = code_index;
    emit(9, 0, 0);

    switch (tokens[token_index].type)
    {
    case 12:
        token_index++;
        break;
    case 1:
    case 6:
    case 7:
    case 8:
    case 9:
    case 11:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 20:
    case 22:
        print_parser_error(12, 0); // sometimes non-stopping
        error = 1;
        break;
    default:
        print_parser_error(12, 0); // sometimes non-stopping
        error = -1;
        return;
    }

    statement();

    emit(8, 0, jmp_index);

    code[jpc_index].m = code_index;
}

void read()
{
    int bool = 0, emit_level = -1, m = -1;

    switch (tokens[token_index].type)
    {
    case 1:
        bool = 1;
        break;
    case 8:
    case 17:
    case 20:
    case 22:
        print_parser_error(2, 5); // sometimes non-stopping
        error = 1;
        break;
    default:
        print_parser_error(2, 5); // sometimes non-stopping
        error = -1;
        return;
    }

    if (bool)
    {
        char name[12];
        strcpy(name, tokens[token_index].identifier_name);

        int index = find_symbol(tokens[token_index].identifier_name, 2);

        /*  If all kinds == -1, no identifier by that name  */
        if (find_symbol(name, 1) == -1 && index == -1 &&
            find_symbol(name, 3) == -1)
        {
            print_parser_error(8, 3); // always non-stopping
            error = 1;
        }

        /*  Identifier by the name, but not of the right type  */
        else if (index == -1)
        {
            print_parser_error(13, 0); // always non-stopping
            error = 1;
        }

        /*  Identifier var found  */
        else
        {
            symbol s = table[index];
            emit_level = level - s.level;
            m = s.address;
        }

        ++token_index;
    }

    emit(10, 0, 2);
    emit(4, emit_level, m);
}

void write()
{
    expression();

    if (error == -1)
        return;

    emit(10, 0, 1);
}

void def()
{
    int bool = 1, index = 0;
    char name[12];
    strcpy(name, tokens[token_index].identifier_name);

    if (tokens[token_index].type != 1)
    {
        print_parser_error(2, 6); // sometimes non-stopping
        bool = 0;

        if (tokens[token_index].type == 21)
            error = 1;

        else
        {
            error = -1;
            return;
        }
    }

    if (bool)
    {
        index = find_symbol(tokens[token_index].identifier_name, 3);

        /*  If all kinds == -1, no identifier by that name  */
        if (find_symbol(name, 1) == -1 && find_symbol(name, 2) == -1 && index == -1)
        {
            bool = 0;
            print_parser_error(8, 4); // always non-stopping
            error = 1;
        }

        /*  Identifier by the name, but not of the right type  */
        else if (index == -1)
        {
            bool = 0;
            print_parser_error(14, 0); // always non-stopping
            error = 1;
        }

        /*  Checks that procedure belongs to current parent procedure  */
        if (bool && (table[index].level != level))
        {
            bool = 0;
            print_parser_error(22, 0); // always non-stopping
            error = 1;
        }

        if (bool &&table[index].address != -1)
        {
            bool = 0;
            print_parser_error(23, 0); // always non-stopping
            error = 1;
        }

        ++token_index;
    }

    // printf("\n%d\n", tokens[token_index].type);

    switch (tokens[token_index].type)
    {
    case 21:
        ++token_index;
        break;
    case 1:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 9:
    case 11:
    case 13:
    case 14:
    case 15:
    case 16:
    case 22:
        print_parser_error(15, 0); // sometimes non-stopping
        error = 1;
        break;
    default:
        print_parser_error(15, 0); // sometimes non-stopping
        error = -1;
        return;
    }

    int jmp_index = code_index;
    emit(8, 0, 0);

    if (bool)
        table[index].address = code_index;

    block();

    if (code[code_index - 1].op != 6)
        emit(6, 0, 0);

    code[jmp_index].m = code_index;

    switch (tokens[token_index].type)
    {
    case 22:
        ++token_index;
        break;
    case 8:
    case 17:
    case 20:
        print_parser_error(16, 0); // sometimes non-stopping
        error = 1;
        break;
    default:
        print_parser_error(16, 0); // sometimes non-stopping
        error = -1;
        return;
    }
}

void return_func()
{
    if (level)
        emit(6, 0, 0);

    else
        emit(10, 0, 3);
}

/*  Condition  */
void condition()
{
    int operation = 0;

    expression();

    switch (tokens[token_index].type)
    {
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
        operation = tokens[token_index++].type;
        break;
    case 1:
    case 2:
    case 32:
        print_parser_error(17, 0); // non-stopping
        error = 1;
        break;
    default:
        print_parser_error(17, 0); // stopping
        error = -1;
        return;
    }

    expression();

    switch (operation)
    {
    case 23:
        emit(2, 0, 5);
        break;
    case 24:
        emit(2, 0, 6);
        break;
    case 25:
        emit(2, 0, 7);
        break;
    case 26:
        emit(2, 0, 8);
        break;
    case 27:
        emit(2, 0, 9);
        break;
    case 28:
        emit(2, 0, 10);
        break;
    default:
        emit(2, 0, -1);
        return;
    }
}

/*  Expression  */
void expression()
{
    token_type t;
    term();

    t = tokens[token_index].type;
    while (t == 19 || t == 29)
    {
        ++token_index;
        term();

        if (t == 19)
            emit(2, 0, 2);
        else
            emit(2, 0, 1);

        t = tokens[token_index].type;
    }
}

/*  Term  */
void term()
{
    token_type t;
    factor();

    t = tokens[token_index].type;
    while (t == 30 || t == 31)
    {
        ++token_index;
        factor();

        if (t == 30)
            emit(2, 0, 3);
        else
            emit(2, 0, 4);

        t = tokens[token_index].type;
    }
}

/*  Factor  */
void factor()
{
    switch (tokens[token_index].type)
    {
    case 1:
        factor_ident();
        ++token_index;
        break;
    case 2:
        emit(1, 0, tokens[token_index++].number_value);
        break;
    case 32:
        factor_left_parenthesis();
        break;
    case 8:
    case 10:
    case 12:
    case 17:
    case 19:
    case 20:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
        print_parser_error(20, 0); // sometimes non-stopping
        error = 1;
        break;
    default:
        print_parser_error(20, 0); // sometimes non-stopping
        error = -1;
        return;
    }
}

void factor_ident()
{
    int index_const = -1, index_var = -1;

    char name[12];
    strcpy(name, tokens[token_index].identifier_name);

    if (find_symbol(name, 1) == -1 && find_symbol(name, 2) == -1)
    {
        if (find_symbol(name, 3) == -1)
            print_parser_error(8, 5); // always non-stopping

        else
            print_parser_error(18, 0); // always non-stopping

        error = 1;
    }

    else if (find_symbol(name, 1) == -1)
    {
        index_var = find_symbol(name, 2);
    }

    else if (find_symbol(name, 2) == -1)
    {
        index_const = find_symbol(name, 1);
    }
    else
    {
        index_const = find_symbol(name, 1);
        index_var = find_symbol(name, 2);
    }

    /*  Determine which instruction to use  */

    /*  Both constant and var found  */
    if (index_const != -1 && index_var != -1)
    {
        /*  Constant level > Var level  */
        if (table[index_const].level > table[index_var].level)
            emit(1, 0, table[index_const].value);

        /*  Var level > Constant level  */
        else
        {
            int l = level - table[index_var].level;
            emit(3, l, table[index_var].address);
        }
    }

    /*  Only a constant  */
    else if (index_const != -1)
        emit(1, 0, table[index_const].value);

    /*  Only a var  */
    else if (index_var != -1)
    {
        int l = level - table[index_var].level;
        emit(3, l, table[index_var].address);
    }
}

void factor_left_parenthesis()
{
    ++token_index;

    expression();

    switch (tokens[token_index].type)
    {
    case 33:
        ++token_index;
        break;
    case 8:
    case 10:
    case 12:
    case 17:
    case 19:
    case 20:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
        print_parser_error(19, 0); // sometimes non-stopping
        error = 1;
        break;
    default:
        print_parser_error(19, 0); // sometimes non-stopping
        error = -1;
        break;
    }
}

/*  Helper Functions  */
int const_follow_statement(token_type t)
{
    switch (t)
    {
    case 1:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 9:
    case 11:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 22:
        return 1;
    default:
        return 0;
    }
}

void emit(int op, int l, int m)
{
    code[code_index].op = op;
    code[code_index].l = l;
    code[code_index].m = m;
    code_index++;
}

void add_symbol(int kind, char name[], int value, int level, int address)
{
    table[table_index].kind = kind;
    strcpy(table[table_index].name, name);
    table[table_index].value = value;
    table[table_index].level = level;
    table[table_index].address = address;
    table[table_index].mark = 0;
    table_index++;
}

void mark()
{
    int i;
    for (i = table_index - 1; i >= 0; i--)
    {
        if (table[i].mark == 1)
            continue;
        if (table[i].level < level)
            return;
        table[i].mark = 1;
    }
}

int multiple_declaration_check(char name[])
{
    int i;
    for (i = 0; i < table_index; i++)
        if (table[i].mark == 0 && table[i].level == level &&
            strcmp(name, table[i].name) == 0)
            return i;
    return -1;
}

int find_symbol(char name[], int kind)
{
    int i;
    int max_idx = -1;
    int max_lvl = -1;
    for (i = 0; i < table_index; i++)
    {
        if (table[i].mark == 0 && table[i].kind == kind &&
            strcmp(name, table[i].name) == 0)
        {
            if (max_idx == -1 || table[i].level > max_lvl)
            {
                max_idx = i;
                max_lvl = table[i].level;
            }
        }
    }
    return max_idx;
}

void print_parser_error(int error_code, int case_code)
{
    switch (error_code)
    {
    case 1:
        printf("Parser Error 1: missing . \n");
        break;
    case 2:
        switch (case_code)
        {
        case 1:
            printf("Parser Error 2: missing identifier after keyword const\n");
            break;
        case 2:
            printf("Parser Error 2: missing identifier after keyword var\n");
            break;
        case 3:
            printf("Parser Error 2: missing identifier after keyword procedure\n");
            break;
        case 4:
            printf("Parser Error 2: missing identifier after keyword call\n");
            break;
        case 5:
            printf("Parser Error 2: missing identifier after keyword read\n");
            break;
        case 6:
            printf("Parser Error 2: missing identifier after keyword def\n");
            break;
        default:
            printf("Implementation Error: unrecognized error code\n");
        }
        break;
    case 3:
        printf("Parser Error 3: identifier is declared multiple times by a "
               "procedure\n");
        break;
    case 4:
        switch (case_code)
        {
        case 1:
            printf("Parser Error 4: missing := in constant declaration\n");
            break;
        case 2:
            printf("Parser Error 4: missing := in assignment statement\n");
            break;
        default:
            printf("Implementation Error: unrecognized error code\n");
        }
        break;
    case 5:
        printf("Parser Error 5: missing number in constant declaration\n");
        break;
    case 6:
        switch (case_code)
        {
        case 1:
            printf("Parser Error 6: missing ; after constant declaration\n");
            break;
        case 2:
            printf("Parser Error 6: missing ; after variable declaration\n");
            break;
        case 3:
            printf("Parser Error 6: missing ; after procedure declaration\n");
            break;
        case 4:
            printf("Parser Error 6: missing ; after statement in begin-end\n");
            break;
        default:
            printf("Implementation Error: unrecognized error code\n");
        }
        break;
    case 7:
        printf("Parser Error 7: procedures and constants cannot be assigned to\n");
        break;
    case 8:
        switch (case_code)
        {
        case 1:
            printf("Parser Error 8: undeclared identifier used in assignment "
                   "statement\n");
            break;
        case 2:
            printf("Parser Error 8: undeclared identifier used in call statement\n");
            break;
        case 3:
            printf("Parser Error 8: undeclared identifier used in read statement\n");
            break;
        case 4:
            printf(
                "Parser Error 8: undeclared identifier used in define statement\n");
            break;
        case 5:
            printf("Parser Error 8: undeclared identifier used in arithmetic "
                   "expression\n");
            break;
        default:
            printf("Implementation Error: unrecognized error code\n");
        }
        break;
    case 9:
        printf("Parser Error 9: variables and constants cannot be called\n");
        break;
    case 10:
        printf("Parser Error 10: begin must be followed by end\n");
        break;
    case 11:
        printf("Parser Error 11: if must be followed by then\n");
        break;
    case 12:
        printf("Parser Error 12: while must be followed by do\n");
        break;
    case 13:
        printf("Parser Error 13: procedures and constants cannot be read\n");
        break;
    case 14:
        printf("Parser Error 14: variables and constants cannot be defined\n");
        break;
    case 15:
        printf("Parser Error 15: missing {\n");
        break;
    case 16:
        printf("Parser Error 16: { must be followed by }\n");
        break;
    case 17:
        printf("Parser Error 17: missing relational operator\n");
        break;
    case 18:
        printf("Parser Error 18: procedures cannot be used in arithmetic\n");
        break;
    case 19:
        printf("Parser Error 19: ( must be followed by )\n");
        break;
    case 20:
        printf("Parser Error 20: invalid expression\n");
        break;
    case 21:
        printf("Parser Error 21: procedure being called has not been defined\n");
        break;
    case 22:
        printf("Parser Error 22: procedures can only be defined within the "
               "procedure that declares them\n");
        break;
    case 23:
        printf("Parser Error 23: procedures cannot be defined multiple times\n");
        break;
    default:
        printf("Implementation Error: unrecognized error code\n");
    }
}

void print_assembly_code()
{
    int i;
    printf("Assembly Code:\n");
    printf("Line\tOP Code\tOP Name\tL\tM\n");
    for (i = 0; i < code_index; i++)
    {
        printf("%d\t%d\t", i, code[i].op);
        switch (code[i].op)
        {
        case LIT:
            printf("LIT\t");
            break;
        case OPR:
            switch (code[i].m)
            {
            case ADD:
                printf("ADD\t");
                break;
            case SUB:
                printf("SUB\t");
                break;
            case MUL:
                printf("MUL\t");
                break;
            case DIV:
                printf("DIV\t");
                break;
            case EQL:
                printf("EQL\t");
                break;
            case NEQ:
                printf("NEQ\t");
                break;
            case LSS:
                printf("LSS\t");
                break;
            case LEQ:
                printf("LEQ\t");
                break;
            case GTR:
                printf("GTR\t");
                break;
            case GEQ:
                printf("GEQ\t");
                break;
            default:
                printf("err\t");
                break;
            }
            break;
        case LOD:
            printf("LOD\t");
            break;
        case STO:
            printf("STO\t");
            break;
        case CAL:
            printf("CAL\t");
            break;
        case RTN:
            printf("RTN\t");
            break;
        case INC:
            printf("INC\t");
            break;
        case JMP:
            printf("JMP\t");
            break;
        case JPC:
            printf("JPC\t");
            break;
        case SYS:
            switch (code[i].m)
            {
            case WRT:
                printf("WRT\t");
                break;
            case RED:
                printf("RED\t");
                break;
            case HLT:
                printf("HLT\t");
                break;
            default:
                printf("err\t");
                break;
            }
            break;
        default:
            printf("err\t");
            break;
        }
        printf("%d\t%d\n", code[i].l, code[i].m);
    }
    printf("\n");
}

void print_symbol_table()
{
    int i;
    printf("Symbol Table:\n");
    printf("Kind | Name        | Value | Level | Address | Mark\n");
    printf("---------------------------------------------------\n");
    for (i = 0; i < table_index; i++)
        printf("%4d | %11s | %5d | %5d | %5d | %5d\n", table[i].kind, table[i].name,
               table[i].value, table[i].level, table[i].address, table[i].mark);
    printf("\n");
}