#include <stdio.h>
#include <stdlib.h>
#include "token.h"

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
        fprintf(stderr, "%d", i);
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
            fprintf(stderr, "%s", tokens[i].lexeme ? tokens[i].lexeme : "(null)");
            fflush(stderr);
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
