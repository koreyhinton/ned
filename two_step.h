#ifndef TWO_STEP_H
#define TWO_STEP_H

#include "token.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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

ModuleHashMap *create_module_hash_map();

void inline_expand(Token *tokens_in, unsigned long n_in, Token **tokens_out, unsigned long *n_out, SymbolHashMap ** symbols_hash_map, int *line);

// void lifetime_resolution(Token *tokens, int n, SymbolTable *symbols);

#endif
