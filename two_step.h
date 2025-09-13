#ifndef TWO_STEP_H
#define TWO_STEP_H

#include "token.h"

typedef struct Symbol
{
    char *name;
    int last_used_line;
    struct Symbol *next;
} Symbol;

typedef struct SymbolHashMap {
    Symbol **symbols;
    unsigned long symbols_len;
    unsigned long symbols_alloc_len;
} SymbolHashMap;

typedef struct Module
{
    char *name;
    Token *tokens;
    unsigned long tokens_len;
    unsigned long tokens_alloc_len;
    struct Module *next;
} Module;

typedef struct ModuleHashMap
{
    Module **modules;
    unsigned long modules_len;
    unsigned long modules_alloc_len;
} ModuleHashMap;

void inline_expand(Token *tokens_in, int n_in, Token **tokens_out, int *n_out, SymbolHashMap **symbol_hash_map);

// void lifetime_resolution(Token *tokens, int n, SymbolTable *symbols);

#endif
