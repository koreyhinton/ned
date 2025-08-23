#include <stdio.h>
#include <stdlib.h>
#include "token.h"

const char *map(int i) {
    switch(i) {
        case 0:
            return "TOKEN_MODULE_IDENTIFIER";
        case 1:
            return "TOKEN_VARIABLE_IDENTIFIER";
        case 2:
            return "TOKEN_COLON";
        case 3:
            return "TOKEN_SPACE";
        case 4:
            return "TOKEN_INDENT";
        case 5:
            return "TOKEN_NEWLINE";
        case 6:
            return "TOKEN_0";
        case 7:
            return "TOKEN_1";
        case 8:
            return "TOKEN_STRING";
        case 9:
            return "TOKEN_NUMBER";
        case 10:
            return "TOKEN_NEGATE";
        case 11:
            return "TOKEN_TYPE_MONAD";
        case 12:
            return "TOKEN_DOT";
        case 13:
            return "TOKEN_AS";
        case 14:
            return "TOKEN_LOOP";
        case 15:
            return "TOKEN_EQUALS";
        default:
            return "tbd";
    }
}

int main(int argc, char *argv[]) {
    // printf("%d\n", argc);
    // printf("%s\n", argv[1]);
    FILE *fp;
    fp = fopen(argv[1], "r");

    fseek(fp, 0, SEEK_END);
    long source_length = ftell(fp);
    char *source = malloc(source_length+1);
    rewind(fp);
    fread(source, 1, source_length, fp);
    /*
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        printf("%c", (char)ch);
    }
    */
    source[source_length] = '\0';
    fclose(fp);

    Token * tokens = tokenize(source);
    printf("#include \"runtime.h\"\n");
    printf("void main(int argc, char *argv[]){");

    for (int i = 0; tokens[i].type != TOKEN_EOP; i++)
    {
        /*
        fprintf(stderr, "%d", i);
        fflush(stderr);
        */
        
        fprintf(stderr, "%s(%s)\n", map(tokens[i].type), tokens[i].lexeme);
        fflush(stderr);


        if (tokens[i].type == TOKEN_INVALID)
        {
            fprintf(stderr, "Error: invalid token found: %s\n", tokens[i].lexeme ? tokens[i].lexeme : "(null)");
            fflush(stderr);
            free(source);
            exit(1);
        }
        else
        {
            // debug to stderr for now
            /*
            fprintf(stderr, "%s", tokens[i].lexeme ? tokens[i].lexeme : "(null)");
            fflush(stderr);
            */
        }
        // printf("token: %s \n", tokens[i].lexeme);
        free(tokens[i].lexeme);
    }

    /* printf("print__number(5.0);"); */
    printf("}\n");
    free(source);

    /*parse(tokenize());*/
    return 0;
}
