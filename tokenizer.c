#include "token.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define INITIAL_TOKEN_CAPACITY 12

int __token_capacity = INITIAL_TOKEN_CAPACITY;

// Grows the `tokens` buffer if ti >= __token_capacity.
// Assumes variables: Token *tokens, int ti, int __token_capacity
#define ENSURE_TOKEN_CAPACITY() \
    do { \
        if (ti >= __token_capacity) { \
            fprintf(stderr, "[debug] growing token buffer to %d\n", __token_capacity * 2); \
            fflush(stderr); \
            __token_capacity *= 2; \
            Token *__realloc_tmp_token_array = realloc(tokens, __token_capacity * sizeof(Token)); \
            if (!__realloc_tmp_token_array) { \
                fprintf(stderr, "Out of memory reallocating tokens\n"); \
                fflush(stderr); \
                exit(1); \
            } \
            tokens = __realloc_tmp_token_array; \
        } \
    } while (0)

#define ENSURE_TOKEN_CAPACITY_P() \
    do { \
        if (*ti >= __token_capacity) { \
            fprintf(stderr, "[debug] growing token buffer to %d\n", __token_capacity * 2); \
            fflush(stderr); \
            __token_capacity *= 2; \
            Token *__realloc_tmp_token_array = realloc(tokens, __token_capacity * sizeof(Token)); \
            if (!__realloc_tmp_token_array) { \
                fprintf(stderr, "Out of memory reallocating tokens\n"); \
                fflush(stderr); \
                exit(1); \
            } \
            tokens = __realloc_tmp_token_array; \
        } \
    } while (0)

#define ENSURE_TOKEN_P_CAPACITY_P() \
    do { \
        if (*ti >= __token_capacity) { \
            fprintf(stderr, "[debug] growing token buffer to %d\n", __token_capacity * 2); \
            fflush(stderr); \
            __token_capacity *= 2; \
            Token *__realloc_tmp_token_array = realloc(*tokens, __token_capacity * sizeof(Token)); \
            if (!__realloc_tmp_token_array) { \
                fprintf(stderr, "Out of memory reallocating *tokens\n"); \
                fflush(stderr); \
                exit(1); \
            } \
            *tokens = __realloc_tmp_token_array; \
        } \
    } while (0)

#define MAKE_INVALID_TOKEN_EXPRESSION_P() (Token){ \
    .type = TOKEN_INVALID, \
    .lexeme = strdup((char[]){*c, '\0'}), \
    .column = *column, \
    .line = line, \
    .indent_level = indent_level \
}

#define MAKE_INVALID_TOKEN_EXPRESSION() (Token){ \
    .type = TOKEN_INVALID, \
    .lexeme = strdup((char[]){*c, '\0'}), \
    .column = column, \
    .line = line, \
    .indent_level = indent_level \
}

/* TOKEN SEQUENCE BUILDS */
typedef struct {
    char expected_char;
    TokenType type;
    bool consume;
} TokenMatch;
bool build_token_seq(const char **pc, int *column, int line, int indent_level,
    Token **tokens, int *ti, const TokenMatch *seq, int count)
{
    const char *c = *pc;

    for (int i = 0; i < count; i++)
    {
        if (seq[i].type == TOKEN_NEWLINE && *c == '\r') { c++; }
        if (*c == '\0' || *c != seq[i].expected_char)
        {
            Token err_token = MAKE_INVALID_TOKEN_EXPRESSION_P();
            ENSURE_TOKEN_P_CAPACITY_P(); (*tokens)[(*ti)++] = err_token;
            return false;
        }
        int col;
        if (seq[i].consume)
        {
            col = (*column)++;
        }
        else
        {
            col = *column;
        }
        Token t = {
            .type = seq[i].type,
            .lexeme = strdup((char[]){*c, '\0'}),
            .line = line,
            .column = col,
            .indent_level = indent_level
        };
        ENSURE_TOKEN_P_CAPACITY_P(); (*tokens)[(*ti)++] = t;
        if (seq[i].consume)
        {
            c++;
        }
    }
    *pc = c;
    return true;
}


/*
    GRAB FUNCTIONS

    A 'grab' means it is expected to fully be there and will otherwise give an invalid token

*/

bool grab_assignment()
{
    // assignment can happen without any = or :=,
    // just a plain bool set or <monad>.exec

}

bool grab_identifier_chain(const char **pc, const int indent_level, Token **tokens, int *ti, int line, int *column, int *grab_count)
{
    *grab_count = 0; // number of identifiers grabbed (can distinguish between var vs mod.var)
    const char *c = *pc;
    const char *c_tok_start = *pc;
    int tok_start_col = *column;
    while (*c != '\0' && ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || *c == '_'))
    {
        c++; (*column)++;
    }

    int len = c - c_tok_start;
    char *lexeme = malloc(len + 1);
    strncpy(lexeme, c_tok_start, len);
    lexeme[len] = '\0';

    Token t = {
        .type = (*c == '.') ? TOKEN_MODULE_IDENTIFIER : TOKEN_VARIABLE_IDENTIFIER,
        .line = line,
        .column = tok_start_col,
        .indent_level = indent_level,
        .lexeme = lexeme
    };

    ENSURE_TOKEN_P_CAPACITY_P(); (*tokens)[(*ti)++] = t;
    (*grab_count)++;

    if (*c == '.')
    {
         Token dot_token = {
            .type = TOKEN_DOT,
            .line = line,
            .column = (*column)++,
            .indent_level = indent_level,
            .lexeme = strdup((char[]){*c, '\0'})
        };
        ENSURE_TOKEN_P_CAPACITY_P(); (*tokens)[(*ti)++] = dot_token;
        c++;

        const char *var_tok_start = c;
        int var_start_col = *column;

        bool valid = (*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || *c == '_';
        if (!valid)
        {
            Token invalid_token =
                MAKE_INVALID_TOKEN_EXPRESSION_P();
            ENSURE_TOKEN_P_CAPACITY_P(); (*tokens)[(*ti)++] = invalid_token;
            *pc = c;
            return false;
        }

        while (*c != '\0' && ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || *c == '_'))
        {
            c++;
            (*column)++;
        }

        int grab2_len = c - var_tok_start;
        char *grab2_lexeme = malloc(grab2_len + 1);
        strncpy(grab2_lexeme, var_tok_start, grab2_len);
        grab2_lexeme[grab2_len] = '\0';
        Token grab2_token = {
            .type = TOKEN_VARIABLE_IDENTIFIER,
            .line = line,
            .column = var_start_col,
            .indent_level = indent_level,
            .lexeme = grab2_lexeme
        };
        ENSURE_TOKEN_P_CAPACITY_P(); (*tokens)[(*ti)++] = grab2_token;
        (*grab_count)++;
    }

    // variable must precede one of the following:
    //     spaces =
    //     spaces :=
    //     :=
    //     < (<monad>)
    if (*c != ' ' && *c != ':' && *c != '<')
    {
        fprintf(stderr, "invalid inside identifier chain found, c = %c", *c);
        fflush(stderr);

        Token invalid_token =
            MAKE_INVALID_TOKEN_EXPRESSION_P();
        ENSURE_TOKEN_P_CAPACITY_P(); (*tokens)[(*ti)++] = invalid_token;
        *pc = c;
        return false;
    }

    // if *c is a '.' then it was a bool var chain, so then:
    //     grab the '.' 
    //     expect and grab the variable
    //     expect:
    //         newline
    //         spaces = spaces (handle spaces fully so it won't interpret as indentation)
    //         spaces := (regular variable default implementation assignment)
    //         := newline (boolean default implementation assignment)
    //         < (monad)
    *pc = c;
    return true;
}

bool grab_boolean_var_chain(const char **pc, const int indent_level, Token *tokens, int *ti, int line, int *column)
{
    // loop{:,\n}
    // module_bool_var{:,\n}
    // module.bool_var{:,\n}
    //
    // Examples (underlined):
    //    premodule:    1:         !_my_bool_
    //    premodule:    1:         i = 1
    //    module:       !_loop_:   _loop_
    //    module:       !_loop_:   _loop_
    //    module:       i as 1:    _premodule.my_bool_
    //    module:       i as 1:    i--
    //    module:       i as 0:    !_premodule.my_bool_
    //    module:       i as 0:    i++
    //
    // What IS NOT a boolean chain:
    //     number var chain:
    //         module:    _premodule.i_ as 1:    _premodule.i_--
    //     type monad chain (even though shaped like one, it has special behavior):
    //         module:    1:                           _1<extmodule>.exec_
    //     Invalid code (type monad accessors are only valid in the last column):
    //        module:     _i as 0<extmodule>.valid_var_:    :

    const char *c = *pc;
    const char *c_tok_start = *pc;
    int tok_start_col = *column;

    if (indent_level == 1 && ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z')) )
    {
        bool module_found = false;
        // boolean chain continues until :
        while (*c != '\0' && ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || *c == '_' || *c == '.'))
        {
            if (*c == ':' && module_found)
            {
                Token t;
                t.line = line;
                t.column = (*column);
                t.indent_level = indent_level;
                int len = c - c_tok_start;
                char *lexeme = malloc(len + 1);
                memcpy(lexeme, c_tok_start, len); // strncpy(lexeme, c_tok_start, len);
                lexeme[len] = '\0';
                if (strcmp(lexeme, "loop") == 0)
                {
                    t.type = TOKEN_LOOP;
                }
                else
                {
                    t.type = TOKEN_VARIABLE_IDENTIFIER;
                }
                c_tok_start = c + 1;
                tok_start_col = *column;

                // c_tok_start++;
                t.lexeme = lexeme;
                ENSURE_TOKEN_CAPACITY_P(); tokens[(*ti)++] = t;
                *pc = c;
                return true; // outer-scope will handle colon
            }
            if (*c == '.' && module_found)
            {
                Token invalid;
                invalid.type = TOKEN_INVALID;
                invalid.lexeme = strdup("Too many dot-accessors for condition expression");
                invalid.column = *column;
                invalid.line = line;
                invalid.indent_level = indent_level;
                ENSURE_TOKEN_CAPACITY_P(); tokens[(*ti)++] = invalid;
                *pc = c;
                return false;
            }
            if (*c == '.' && !module_found)
            {
                int len = c - 1 - c_tok_start;
                char *lexeme = malloc(len + 1);
                strncpy(lexeme, c_tok_start, len);
                lexeme[len] = '\0';
                c_tok_start = c + 1;

                Token t;
                {
                    t.type = TOKEN_MODULE_IDENTIFIER;
                    t.line = line;
                    t.column = tok_start_col;
                    t.indent_level = indent_level;
                    t.lexeme = lexeme;
                }
                ENSURE_TOKEN_CAPACITY_P(); tokens[(*ti)++] = t;

                Token dot;
                {
                    dot.type = TOKEN_DOT;
                    dot.line = line;
                    dot.column = (*column);
                    dot.indent_level = indent_level;
                    dot.lexeme = strdup((char[]){*c, '\0'});
                }
                ENSURE_TOKEN_CAPACITY_P(); tokens[(*ti)++] = dot;

                module_found = true;
            }
            c++; (*column)++;
        }
    }

    /* I think last column is too complex to know a boolean chain is present without backtracking,
       so use the grab_identifier_chain() instead
    if (indent_level == 2 && ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z')) )
    {
        // (boolean-like) external module or
        // boolean chain continues until [\s].*$ (without '=' or it would be an assignment)
        // unless part of a boolean default implementation [!]var_name:=

        // non-boolean assignment with [mod.]x = [mod.]y
        // or default implementation x := "y", x := 3
        //     (but not boolean default implementation!: is_x:= or !is_x:=)

    }*/

    return false;
}

int grab_indent(const char **pc, int indent_level, Token **tokens, int *ti, int line, int *column)
{
    const char *c = *pc;
    const char *c_start = *pc;
    int tab_count = 0;
    int new_indent_level = 0;


    while (*c == ' ' || *c == '\t')
    {
        Token t;
        t.type = TOKEN_INDENT;
        t.line = line;
        t.column = (*column);
        t.indent_level = new_indent_level;

        if (*c == '\t')
        {
            // fprintf(stderr, "\\t");
            // fflush(stderr);
            t.lexeme = strdup("\\t");
            new_indent_level += 1;
            c++; (*column)++;
        }
        else
        {
            // space
            int space_count = 0;
            while (*c == ' ' && space_count < 4) {
                // fprintf(stderr, "\\s");
                // fflush(stderr);
                space_count++;
                c++; (*column)++;
            }
            if (space_count != 4) {
                return -1;
            }
            t.lexeme = strdup("\\s\\s\\s\\s");
            new_indent_level += 1;
        }

        ENSURE_TOKEN_P_CAPACITY_P(); (*tokens)[(*ti)++] = t;
        fprintf(stderr, "increased capacity, c=%c\n", *c);
        fflush(stderr);
    }
    if (new_indent_level > 2/* || new_indent_level - indent_level > 1*/)
    {
        // since indent level resets upon encountering newline, we can't test
        // if the next line only incremented by 1
        fprintf(stderr, "new indent level error: %d,%d\n", indent_level, new_indent_level);
        fflush(stderr);

        return -1;
    }

    *pc = c;
    return new_indent_level;
}


Token grab_control_module(const char **pc, int line, int *column)
{
    const char *c = *pc;
    const char *c_start = *pc;
    int start_column = *column;

    bool at_least_once = false;
    while (*c != '\0' && ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || *c == '_'))
    {
        at_least_once = true;
        c++;
        (*column)++;
    }
    if (*c == ':' && at_least_once)
    {
        // fprintf(stderr, ":");
        // fflush(stderr);
        int len = c - c_start;
        char *lexeme = malloc(len + 1);
        strncpy(lexeme, c_start, len);
        lexeme[len] = '\0';
        // *pc = c + 1;  // skip over colon
        *pc = c; // *pc == ':'
        (*column)++;
        Token t;
        t.type = TOKEN_MODULE_IDENTIFIER;
        t.lexeme = lexeme;
        t.indent_level = 0;
        t.line = line;
        t.column = start_column;
        return t;
    }
    Token invalid;
    invalid.type = TOKEN_INVALID;
    invalid.lexeme = NULL;
    invalid.column = *column;
    invalid.line = line;
    invalid.indent_level = 0;
    return invalid;
}

Token *tokenize(const char *source_lang)
{
    int line = 1; // 1-based (editor-style numbering)
    int column = 1; // 1-based (editor-style numbering)

    int ti = 0; // token index
    //static Token tokens[1024];
    Token *tokens = malloc(INITIAL_TOKEN_CAPACITY * sizeof(Token));
    if (!tokens) {
        fprintf(stderr, "Out of memory allocating initial token buffer\n");
        exit(1);
    }

    int indent_level = 0; // 0, 1, 2
    const char *c = source_lang;
    while (*c != '\0')
    {
        if (*c != '\t' && *c != '\n' && *c != ' ')
        {
            fprintf(stderr, "^%c*\n", *c);
            fflush(stderr);
        }

        /*
            STATE TRANSITIONS

            '\n' => indentation reset
                indent_level = 0
                column       = 1
                line++

            '\t' or '    ' => indentation aggregate (0,1,2)
                indent_level++

            other => start of a code expression

        */

        if (indent_level == 1 && ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (*c == '_')))
        {
            /*
                        handled this iteration
                   ____/____    or:  ___\___________
                  /         \       /               \
                \t{varchain}:\n   \t{varchain} as 0 1:\n
                  ^
                  |       \
                   *c      \
                            loop keyword gets its own token

            */

            int grab_count;
            if (!grab_identifier_chain(&c, indent_level, &tokens, &ti, line, &column, &grab_count))
            {
fprintf(stderr, "tokenize end, ti=%d\n", ti);
fflush(stderr);
                goto end;
            }

            if (grab_count == 1 && tokens[ti-1].lexeme == "loop")
            {
                tokens[ti-1].type = TOKEN_LOOP;
            }

            if (strncmp(c, " as ", 4) == 0)
            {
                Token t_sp1 = {
                    .type = TOKEN_SPACE,
                    .lexeme = strdup((char[]){*c, '\0'}),
                    .line = line,
                    .column = column++,
                    .indent_level = indent_level
                };
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t_sp1;
                c++;

                Token t_as = {
                    .type = TOKEN_AS,
                    .lexeme = strdup("as"),
                    .line = line,
                    .column = column++,
                    .indent_level = indent_level
                };
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t_as;
                c++; c++;

                Token t_sp2 = {
                    .type = TOKEN_SPACE,
                    .lexeme = strdup((char[]){*c, '\0'}),
                    .line = line,
                    .column = column++,
                    .indent_level = indent_level
                };
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t_sp2;
                c++;

                if (*c != '0' && *c != '1')
                {
                    Token t_inv = {
                        .type = TOKEN_INVALID,
                        .lexeme = strdup((char[]){*c, '\0'}),
                        .line = line,
                        .column = column,
                        .indent_level = indent_level
                    };
                    ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t_inv;
                    goto end;
                }

                TokenType token_type = TOKEN_1;
                if (*c == '0')
                {
                    token_type = TOKEN_0;
                }
                Token t_bit = {
                    .type = token_type,
                    .lexeme = strdup((char[]){*c, '\0'}),
                    .line = line,
                    .column = column++,
                    .indent_level = indent_level
                };
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t_bit;
                c++;
            }
            if (*c != ':')
            {
                Token t_inv = {
                    .type = TOKEN_INVALID,
                    .lexeme = strdup((char[]){*c, '\0'}),
                    .line = line,
                    .column = column,
                    .indent_level = indent_level
                };
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t_inv;
                goto end;
            }

            Token colon_t;
            colon_t.type = TOKEN_COLON;
            colon_t.line = line;
            colon_t.column = column++;
            colon_t.lexeme = strdup((char[]){*c, '\0'}); // strdup(":");
            colon_t.indent_level = 1;
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = colon_t;
            // note: `if (*c == ' ' || *c == '\t')` block will increment indent_level

            fprintf(stderr, "end of condition, c=%c\n", *c);
            fflush(stderr);

            /* early continue (sandwiched around error checks): */
            c++;
            if (*c == '=')
            {
                Token invalid;
                invalid.type = TOKEN_INVALID;
                invalid.lexeme = strdup("= (error: default implementation notation is not valid at the condition level)");
                invalid.column = column;
                invalid.line = line;
                invalid.indent_level = 0;
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = invalid;
                goto end;
            }
            if (*c != '\n')
            {
                char tmp[64];
                snprintf(tmp, sizeof(tmp), "%c (error: expected newline)", *c);
                Token invalid;
                invalid.type = TOKEN_INVALID;
                invalid.lexeme = strdup(tmp);
                invalid.column = column;
                invalid.line = line;
                invalid.indent_level = 0;
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = invalid;
                goto end;
            }
            continue;
        }

        if (indent_level == 1 && *c == '!')
        {
            /*
                        handled this iteration
                   ____/_____
                  /          \
                \t!{varchain}:\n
                  ^ 
                  |
                   *c
            */

            Token t;
            t.type = TOKEN_NEGATE;
            t.lexeme = strdup((char[]){*c, '\0'});
            t.line = line;
            t.column = column++;
            t.indent_level = indent_level;
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t;
            c++;

            int grab_count;
            if (!grab_identifier_chain(&c, indent_level, &tokens, &ti, line, &column, &grab_count))
            {
fprintf(stderr, "tokenize end, ti=%d\n", ti);
fflush(stderr);
                goto end;
            }
            Token colon_t;
            colon_t.type = TOKEN_COLON;
            colon_t.line = line;
            colon_t.column = column++;
            colon_t.lexeme = strdup((char[]){*c, '\0'}); // strdup(":");
            colon_t.indent_level = 1;
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = colon_t;
            // note: `if (*c == ' ' || *c == '\t')` block will increment indent_level

            fprintf(stderr, "end of condition, c=%c\n", *c);
            fflush(stderr);

            /* early continue (sandwiched around error checks): */
            c++;
            if (*c == '=')
            {
                Token invalid;
                invalid.type = TOKEN_INVALID;
                invalid.lexeme = strdup("= (error: default implementation notation is not valid at the condition level)");
                invalid.column = column;
                invalid.line = line;
                invalid.indent_level = 0;
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = invalid;
                goto end;
            }
            if (*c != '\n')
            {
                char tmp[64];
                snprintf(tmp, sizeof(tmp), "%c (error: expected newline)", *c);
                Token invalid;
                invalid.type = TOKEN_INVALID;
                invalid.lexeme = strdup(tmp);
                invalid.column = column;
                invalid.line = line;
                invalid.indent_level = 0;
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = invalid;
                goto end;
            }
            continue;
        }
        if (indent_level == 1 && *c == '1')
        {
            Token t = {
                .type = TOKEN_1,
                .lexeme = strdup((char[]){*c, '\0'}),
                .line = line,
                .column = column++,
                .indent_level = indent_level
            };
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t;
            c++;
            if (*c != ':')
            {
                Token invalid = {
                    .type = TOKEN_INVALID,
                    .lexeme = strdup((char[]){*c, '\0'}),
                    .column = column,
                    .line = line,
                    .indent_level = indent_level
                };
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = invalid;
                goto end;
            }
            else
            {
                fprintf(stderr, "got to the 1: part. %d\n", indent_level);
                fflush(stderr);
                Token t = {
                    .type = TOKEN_COLON,
                    .lexeme = strdup((char[]){*c, '\0'}),
                    .column = column++,
                    .line = line,
                    .indent_level = indent_level
                };
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t;
            }

            /* early continue (sandwiched around error checks): */
            c++;
            if (*c == '=')
            {
                Token invalid;
                invalid.type = TOKEN_INVALID;
                invalid.lexeme = strdup("= (error: default implementation notation is not valid at the condition level)");
                invalid.column = column;
                invalid.line = line;
                invalid.indent_level = 0;
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = invalid;
                goto end;
            }
            if (*c != '\n')
            {
                char tmp[64];
                snprintf(tmp, sizeof(tmp), "%c (error: expected newline)", *c);
                Token invalid;
                invalid.type = TOKEN_INVALID;
                invalid.lexeme = strdup(tmp);
                invalid.column = column;
                invalid.line = line;
                invalid.indent_level = 0;
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = invalid;
                goto end;
            }
            continue;
        }
        if (indent_level == 1 && *c == '0')
        {
            Token t = {
                .type = TOKEN_0,
                .lexeme = strdup((char[]){*c, '\0'}),
                .line = line,
                .column = column++,
                .indent_level = indent_level
            };
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t;
            c++;
            if (*c != ':')
            {
                Token invalid = {
                    .type = TOKEN_INVALID,
                    .lexeme = strdup((char[]){*c, '\0'}),
                    .column = column,
                    .line = line,
                    .indent_level = indent_level
                };
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = invalid;
                goto end;
            }
            else
            {
                fprintf(stderr, "got to the 1: part. %d\n", indent_level);
                fflush(stderr);
                Token t = {
                    .type = TOKEN_COLON,
                    .lexeme = strdup((char[]){*c, '\0'}),
                    .column = column++,
                    .line = line,
                    .indent_level = indent_level
                };
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t;
            }

            /* early continue (sandwiched around error checks): */
            c++;
            if (*c == '=')
            {
                Token invalid;
                invalid.type = TOKEN_INVALID;
                invalid.lexeme = strdup("= (error: default implementation notation is not valid at the condition level)");
                invalid.column = column;
                invalid.line = line;
                invalid.indent_level = 0;
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = invalid;
                goto end;
            }
            if (*c != '\n')
            {
                char tmp[64];
                snprintf(tmp, sizeof(tmp), "%c (error: expected newline)", *c);
                Token invalid;
                invalid.type = TOKEN_INVALID;
                invalid.lexeme = strdup(tmp);
                invalid.column = column;
                invalid.line = line;
                invalid.indent_level = 0;
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = invalid;
                goto end;
            }
            continue;
        }
        /*if (indent_level == 1 && *c != '\n' && *c != '\r' && *c != '\t' && *c != ' ')
        {
            //??
        }*/
        if (*c == '\r')
        {
            c++;
            continue;
        }
        if (*c == '\n')
        {
            // fprintf(stderr, "newline\n");
            // fflush(stderr);
            Token t;
            t.type = TOKEN_NEWLINE;
            t.lexeme = strdup((char[]){*c, '\0'});
            t.line = line++;
            t.column = column;
            column = 1;
            t.indent_level = indent_level;
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t;
            indent_level = 0;
            c++;
            continue;
        }
        if (indent_level == 0 && ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z')) )
        {
            // fprintf(stderr, "control_module");
            // fflush(stderr);
            Token control_t = grab_control_module(&c, line, &column);
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = control_t;
            if (control_t.type == TOKEN_INVALID)
            {
                goto end;
                // break;
            }
            Token colon_t;
            colon_t.type = TOKEN_COLON;
            colon_t.line = line;
            colon_t.column = column++;
            colon_t.lexeme = strdup((char[]){*c, '\0'});
            colon_t.indent_level = 0;
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = colon_t;
            c++;

            if (*c == '=')
            {
                // fprintf(stderr, "=");
                // fflush(stderr);

                // default implementation declaration
                Token t;
                t.type = TOKEN_EQUALS;
                t.lexeme = strdup((char[]){*c, '\0'});
                t.line = line;
                t.column = column++;
                t.indent_level = 0;
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t;
                c++;
            }
            if (*c == '\r') { /*skip. c++;*/ }
            if (*c != '\n')
            {
                Token t;
                t.type = TOKEN_INVALID;
                t.lexeme = strdup((char[]){*c, '\0'});
                t.line = line;
                t.column = column;
                t.indent_level = 0;
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t;
                goto end;
            }
            // add the newline token:
            Token t;
            t.type = TOKEN_NEWLINE;
            t.lexeme = strdup((char[]){*c, '\0'});
            t.line = line++;
            t.column = column;
            column = 1;
            t.indent_level = 0;
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t;
            c++;
            continue;
        }
        if (indent_level == 2 && *c == '!')
        {
            Token t = {
                .type = TOKEN_NEGATE,
                .lexeme = strdup((char[]){*c, '\0'}),
                .line = line,
                .column = column++,
                .indent_level = 0
            };
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = t;
            c++;
            continue;
        }
        if (indent_level == 2 && (*c >= '0' && *c <= '9') )
        {
            fprintf(stderr, "got to number (after assignment), %c\n", *c);
            fflush(stderr);
        }
        if (indent_level == 2 && *c == '`')
        {

        }
        if (indent_level == 2 && ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (*c == '_')) )
        {
            // todo: implement these "preceding" checks in the parser
            // what's allowed to come first in the last indent is
            // either:
            //     a module (preceding a dot, then next must be a variable)
            //     or a variable (preceding [ws]= or := or [ws]:= or <monad>)
            //     or a ! (preceding a variable or module)
            //     or a number (preceding a monad: 1<print>)
            //     or a string (preceding a monad: `test`<print>)
            // but this conditioned block only handles module or variable identifier

            fprintf(stderr, "got to identifier chain block\n");
            fflush(stderr);
            int grab_count;
            if (!grab_identifier_chain(&c, indent_level, &tokens, &ti, line, &column, &grab_count))
            {
                goto end;
            }


            if (*c == '<')
            {
                // todo: grab monad and then you have to possible grab variable after that:
                //
                // these examples are valid:
                //     i<print>.exec
                //     fw = "/temp/path.txt"<file_writer>
                //     fw.content = `text`
                //     fw.exec
                // but this is one NOT valid (since it can't ever invoke .exec, should have
                //                            stored monad to a variable):
                //     "path"<file_writer>.content = ``
                //
                // todo: let it continue outside if statement to add on assignment and rhs:
                //     fw.content_ = `text`_
            }

            if (*c == '\r') { c++; }
            if (*c == '\n')
            {
                // plain boolean assignment expression
                continue; // newline handled on next iteration
            }

            /* :=\n */
            if (*c == ':')
            {
                TokenMatch colon_equals_nl_build[] = {
                    { ':', TOKEN_COLON, true },
                    { '=', TOKEN_EQUALS, true },
                    { '\n', TOKEN_NEWLINE, false } // '\n' not consumed
                };
                if (!build_token_seq(&c, &column, line, indent_level, &tokens, &ti, colon_equals_nl_build, 3))
                {
                    goto end;
                }
                continue; // '\n' consumed on the next loop iteration
            }


            if (*c != ' ')
            {
                Token err_token =
                    MAKE_INVALID_TOKEN_EXPRESSION();
                ENSURE_TOKEN_CAPACITY(); tokens[ti++] = err_token;
                goto end;
            }

            Token space_t = {
                .type = TOKEN_SPACE,
                .lexeme = strdup((char[]){*c, '\0'}),
                .line = line,
                .column = column++,
                .indent_level = indent_level
            };
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = space_t;
            c++;

            /* :=\s build */
            if (*c == ':')
            {
                TokenMatch colon_equals_sp_build[] = {
                    { ':', TOKEN_COLON, true },
                    { '=', TOKEN_EQUALS, true },
                    { ' ', TOKEN_NEWLINE, true }
                };
                if (!build_token_seq(&c, &column, line, indent_level, &tokens, &ti, colon_equals_sp_build, 3))
                {
                    goto end;
                }
                continue; // todo: consume rhs
            }

            if (*c == '=')
            {
                TokenMatch equals_sp_build[] = {
                    { '=', TOKEN_EQUALS, true },
                    { ' ', TOKEN_SPACE, true }
                };
                if (!build_token_seq(&c, &column, line, indent_level, &tokens, &ti, equals_sp_build, 2))
                {
                    goto end;
                }
                continue; // todo: consume rhs
            }
            Token err_token =
                MAKE_INVALID_TOKEN_EXPRESSION();
            ENSURE_TOKEN_CAPACITY(); tokens[ti++] = err_token;
            goto end;
        }
        if (*c == ' ' || *c == '\t')
        {
            fprintf(stderr, "start indentation block, c=%c\n", *c);
            fflush(stderr);

            int result = grab_indent(&c, indent_level, &tokens, &ti, line, &column);
            if (result == -1)
            {
                fprintf(stderr, "got to indentation block error: %d\n", indent_level);
                fflush(stderr);
                
            }
            indent_level = result;
            fprintf(stderr, "got to indentation block: %d\n", indent_level);
            fflush(stderr);
            continue; // grab_indent incremented c, right?
        }
        c++;
    }

end:
    ENSURE_TOKEN_CAPACITY(); tokens[ti].type = TOKEN_EOP; // must have an end token to end the calling loop!
    tokens[ti].lexeme = NULL;
    tokens[ti].indent_level = indent_level;
    return tokens;
}
