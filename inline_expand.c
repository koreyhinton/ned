#include "two_step.h"

// djb2
unsigned long hash_string(const char *s)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *s++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}
Symbol* symbol_map_lookup(SymbolHashMap *map, const char *name)
{
    unsigned long idx = hash_string(name) % map->symbols_alloc_len;
    Symbol *s = map->symbols[idx];
    while (s) {
        if (strcmp(s->name, name) == 0)
            return s;
        s = s->next;
    }
    return NULL;
}
Module* module_map_lookup(ModuleHashMap *map, const char *name)
{
    unsigned long idx = hash_string(name) % map->modules_alloc_len;
    Module *m = map->modules[idx];
    while (m) {
        if (strcmp(m->name, name) == 0)
            return m;
        m = m->next;
    }
    return NULL;
}
void symbol_map_insert(SymbolHashMap *map, const char *name, int last_used_line) {
    unsigned long idx = hash_string(name) % map->symbols_alloc_len;

    Symbol *s = map->symbols[idx];
    while (s) {
        if (strcmp(s->name, name) == 0) {
            s->last_used_line = last_used_line;
            return;
        }
        s = s->next;
    }

    Symbol *new_sym = malloc(sizeof(Symbol));
    new_sym->name = strdup(name);
    new_sym->last_used_line = last_used_line;
    new_sym->next = map->symbols[idx];
    map->symbols[idx] = new_sym;
    map->symbols_len++;
}
void module_map_insert(ModuleHashMap *map, const char *name) {
    unsigned long idx = hash_string(name) % map->modules_alloc_len;

    Module *m = map->modules[idx];
    while (m) {
        if (strcmp(m->name, name) == 0) {
            return;
        }
        m = m->next;
    }

    Module *new_mod = malloc(sizeof(Module));
    new_mod->name = strdup(name);
    new_mod->next = map->modules[idx];
    map->modules[idx] = new_mod;
    map->modules_len++;
}


static void append_tokens(Token *in, unsigned long in_len, Token **out, unsigned long *out_len, SymbolHashMap ** symbols_hash_map)
{
    if (in_len == 0) return;
    Token *tmp = realloc(*out, (*out_len + in_len) * sizeof(Token));
    if (!tmp)
    {
        fprintf(stderr, "Out of memory");
        fflush(stderr);
        exit(1);
    }
    *out = tmp;
    memcpy(*out + *out_len, in, in_len*sizeof(Token));
    *out_len += in_len;
}

ModuleHashMap *module_hash_map = NULL;
ModuleHashMap * create_module_hash_map()
{
    ModuleHashMap *temp = malloc(sizeof(ModuleHashMap));
    if (!temp)
    {
        fprintf(stderr, "Out of memory");
        exit(1);
    }
    Module *temp2 = malloc(sizeof(Module) * 10000);
    temp2->tokens = NULL;
    temp2->tokens_alloc_len = 0;
    temp2->tokens_len = 0;
    //temp2->tokens = malloc(sizeof(Token) * 10000);
    if (!temp2)
    {
        fprintf(stderr, "Out of memory");
        exit(1);
    }
    module_hash_map->modules = &temp2;
    return temp;
}
SymbolHashMap * create_symbol_hash_map()
{
    SymbolHashMap *temp = malloc(sizeof(SymbolHashMap));
    if (!temp)
    {
        fprintf(stderr, "Out of memory");
        exit(1);
    }
    Symbol *temp2 = malloc(sizeof(Symbol) * 10000);
    if (!temp2)
    {
        fprintf(stderr, "Out of memory");
        exit(1);
    }
    temp->symbols = NULL;
    temp->symbols_len = 0;
    temp->symbols_alloc_len = 0;
    return temp;
}

void inline_expand(Token *tokens_in, unsigned long n_in, Token **tokens_out, unsigned long *n_out, SymbolHashMap ** symbols_hash_map, int *line)
{
fprintf(stderr, "got to inline expand\n");
fflush(stderr);

    *tokens_out = NULL;
    *n_out = 0;
    char *current_module_name = NULL;
    bool current_is_looping_module = false;

    if (module_hash_map == NULL)
        module_hash_map = create_module_hash_map();
    if (*symbols_hash_map == NULL)
        *symbols_hash_map = create_symbol_hash_map();

fprintf(stderr, "got to inline expand 2\n");
fflush(stderr);


    for (int i = 0; i < n_in; i++)
    {
        Token *tok = &tokens_in[i];

fprintf(stderr, "got to i=%d\n", i);
fflush(stderr);

        // VARIABLE (OR VAR. CHAIN) EXISTING AT THE CONDITION OR RUN LINE LEVEL
        //     STEP 1: SYMBOL UPDATE LIFETIME LINE
        //     STEP 2: TOKEN GETS FULLY QUALIFIED NAME
        //     STEP 3: FALL THROUGH TO APPEND A A SINGLE OUTPUT TOKEN TO MODMAP
        if (tok->type == TOKEN_VARIABLE_IDENTIFIER && (tok->indent_level == 1 || tok->indent_level == 2))
        {
            // a reference (even at condition level),
            // should update symbol line (and gen. code won't free it yet)
            int new_alloc_len = strlen(current_module_name)+strlen(tok->lexeme)+2+1;
            char *full_qual_varname = (char *)malloc(new_alloc_len);
            strcpy(full_qual_varname, current_module_name);
            strcat(full_qual_varname, "__");
            strcat(full_qual_varname, tok->lexeme);
            Symbol *sym = symbol_map_lookup(*symbols_hash_map, full_qual_varname);

            if (!sym)
            {
                symbol_map_insert(
                    *symbols_hash_map, full_qual_varname, *line);
                sym = symbol_map_lookup(
                    *symbols_hash_map, full_qual_varname);
            }
            sym->last_used_line = *line;
            tok->lexeme = realloc(tok->lexeme, new_alloc_len);
            if (!tok->lexeme)
            {
                fprintf(stderr, "Out of memory");
                exit(1);
            }
            free(tok->lexeme);
            tok->lexeme = full_qual_varname;
        }
        if (tok->type == TOKEN_MODULE_IDENTIFIER &&
            (tok->indent_level == 1 || tok->indent_level == 2) &&
            i + 2 < n_in && tokens_in[i + 1].type == TOKEN_DOT &&
            tokens_in[i + 2].type == TOKEN_VARIABLE_IDENTIFIER)
        {
            // consolidate into a fully qualified variable
            int new_alloc_len = strlen(tok->lexeme)+strlen(tok->lexeme)+2+1;
            char *full_qual_varname = (char *)malloc(new_alloc_len);
            strcpy(full_qual_varname, current_module_name);
            strcat(full_qual_varname, "__");
            strcat(full_qual_varname, tokens_in[i+2].lexeme);
            Symbol *sym = symbol_map_lookup(*symbols_hash_map, full_qual_varname);

            if (!sym)
            {
                symbol_map_insert(
                    *symbols_hash_map, full_qual_varname, *line);
                sym = symbol_map_lookup(
                    *symbols_hash_map, full_qual_varname);
            }
            sym->last_used_line = *line;

            tok->lexeme = realloc(tok->lexeme, new_alloc_len);
            if (!tok->lexeme)
            {
                fprintf(stderr, "Out of memory");
                exit(1);
            }
            free(tok->lexeme);
            tok->lexeme = full_qual_varname;
            i += 2;
        }

        // NEXT MODULE IDENTIFIED AT MODULE-LEVEL
        //     STEP 1: APPEND ALL TOKENS FROM PREVIOUS MODULE
        //     STEP 2: CONDITIONALLY EITHER REUSE THE NEW MODULE FROM MOD MAP
        //             ELSE JUST UPDATE THE CURRENT MODULE NAME
        //     STEP 3: CONTINUE EARLY, MODULES BY THEMSELVES AREN'T VALID
        //             PHASE TWO (LIFETIME RES. PARSE) TOKENS
        if (tok->type == TOKEN_MODULE_IDENTIFIER && tok->indent_level == 0 &&
            (current_module_name == NULL || strcmp(current_module_name, tok->lexeme) != 0))
        {
            char *module_name = strdup(tok->lexeme);
            Module *module = module_map_lookup(module_hash_map, module_name);
            if (!module)
            {
                module = module_map_lookup(module_hash_map, current_module_name);
                if (!module && current_module_name != NULL)
                {
                    fprintf(stderr, "Expected module");
                    fflush(stderr);
                    exit(1);
                }
                append_tokens(module->tokens, module->tokens_len, tokens_out, n_out, symbols_hash_map);
                free(current_module_name);
                current_module_name = module_name;
                current_is_looping_module = false; // reset
                continue;
            }
            free(current_module_name);

            // Notice that toggling between previous modules repeatedly,
            // will require ALWAYS updating the current_module_name so later
            // iterations will distinguish between A) reuse repeats:
            //     module_1:
            //         1:
            //             :
            //     module_2
            //         1:
            //             :
            //     module_1:
            //     module_2:
            //     module_1:
            //     module_3:
            //         1:
            //             :
            // Versus B) ignored module name repeat:
            //     module_1:\t1:\t:
            //     module_1:\t1:\t:
            current_module_name = module_name;

            // reuse
            Token *expanded = NULL;
            unsigned long expanded_len = 0;
            inline_expand(module->tokens, module->tokens_len, &expanded, &expanded_len, symbols_hash_map, line);
            append_tokens(expanded, expanded_len, tokens_out, n_out, symbols_hash_map);
            free(expanded);
            continue;
        }

        // SAME EXISTING MODULE RE-STATED (AT MODULE-LEVEL)
        // AND IGNORED BY CONTINUING EARLY
        if (tok->type == TOKEN_MODULE_IDENTIFIER && tok->indent_level == 0 &&
            (current_module_name != NULL && strcmp(current_module_name, tok->lexeme) == 0))
        {
            continue; // stays within the same module
        }

        // ALL VALID PHASE TWO (LIEFTIME RESOLUTION PARSE) TOKENS FALLTHROUGH
        //     STEP 1: GET OR CREATE MODULE IN MAP
        //     STEP 2: APPEND TOKENS TO THE MODULE MAP ENTRY
        Module *module = module_map_lookup(module_hash_map, current_module_name);
        if (!module)
        {
            ModuleHashMap *map = malloc(sizeof(ModuleHashMap));
            module_map_insert(map, current_module_name);
            module = module_map_lookup(module_hash_map, current_module_name);
            if (!module)
            {
                fprintf(stderr, "Expected module");
                fflush(stderr);
                exit(1);
            }
        }
        Token tmp[1] = {*tok};
        //int mod_tokens_len = module->tokens_len;
        //int *mod_tokens_len_p = &mod_tokens_len;
        append_tokens(tmp, 1, &module->tokens, &module->tokens_len, symbols_hash_map);
        if (tok->type == TOKEN_LOOP && tok->indent_level == 2)
            current_is_looping_module = true;
        if (tok->type == TOKEN_NEWLINE)
        {
            (*line)++;
        }
    }
}
