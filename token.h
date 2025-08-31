#ifndef TOKEN_H
#define TOKEN_H

// IMPORTANT:
//    Enum adds or removes will require an update in main.c:map()
typedef enum {
    TOKEN_MODULE_IDENTIFIER, //  col1*       col2*         col3*
                             // |------------|-------------|-------------------|
                             //  m1:           1:            var = `a`
                             //  m2:           1:            mPrint.exec
                             //  m2:           mPrint.exec   a = m1.var
                             //  m1:
                             //  m2:
                             //  m3:           0:            m3 re-ran m1 and m2
                             // |------------|-------------|-------------------|
                             //   m*           m*            m*
                             //   m1:m2:m3 (consecutive re-call)

    TOKEN_VARIABLE_IDENTIFIER, // col1        col2*         col3*
                             // |------------|-------------|-------------------|
                             //  module_name: bool_var:     !bool_var
                             //  module_name: num_var as 0: num_var++
                             //  module_name: 1:            str_var = ``
                             // |------------|-------------|-------------------|
                             //                bool/num_var  str_var

    TOKEN_COLON,             //  col1*        col2*         col3*
                             // |------------|-------------|-------------------|
                             //  module:       1:            :
                             // |------------|-------------|-------------------|
                             //  :             :             : (no-op)         |

    TOKEN_SPACE,             //  col1         col2          col3*
                             // |------------|-------------|-------------------|
                             //  ext_mod:=     1:            var := ``
                             //  module:       1:            var = ``
                             // |------------|-------------|-------------------|
                             //                                <spc>:=<spc>    |
                             //                              <spc>=<spc>       |

    TOKEN_INDENT,            //  col1*        col2*         col3
                             // |------------|-------------|-------------------|
                             //  mod:         0:            comment
                             // |------------|-------------|-------------------|
                             //      <tab>      <tab>
                             //      <spc>*4    <spc>*4

    TOKEN_NEWLINE,           //  col1         col2          col3*
                             // |------------|-------------|-------------------|
                             //  mod:         1:            done
                             // |------------|-------------|-------------------|
                             //                                       <nl>

    TOKEN_0,                 //  col1         col2*         col3
    TOKEN_1,                 // |------------|-------------|-------------------|
                             //  mod:          1:           i = 1
                             //  mod:          0:           comment
                             //  mod:          i as 1:      yes
                             //  mod:          yes:         i--
                             //  mod:          i as 0:      done
                             // |------------|-------------|-------------------|
                             //                0
                             //                    1

    TOKEN_STRING,            //  col1         col2          col3*
                             // |------------|-------------|-------------------|
                             //  mod:         1:            s = `example`
                             //  mod2:        1:            mod.s = `overwrite`
                             //  mod2:        1:            `str`<print>.exec
                             // |------------|-------------|-------------------|
                             //                              ` ... `

    TOKEN_NUMBER,            //  col1         col2*         col3*
                             // |------------|-------------|-------------------|
                             //  mod:         1:            n = 2.5
                             //  mod:         1:            n = 2
                             //  mod:         1:            n = 2.
                             //  mod:         1:            n--
                             //  mod:         n as 1:       n--
                             //  mod:         n as 0:       done
                             // |------------|-------------|-------------------|
                             //                <n> as 1      _ = <n>
                             //                <n> as 0      _ = <n>.[<n>]

    TOKEN_NEGATE,            //  col1         col2*         col3*
                             // |------------|-------------|-------------------|
                             //  mod:         1:            bool_var
                             //  mod:         bool_var:     !bool_var
                             //  mod:         !bool_var:    done
                             // |------------|-------------|-------------------|
                             //                !bool_var    !bool_var

    TOKEN_TYPE_MONAD,        //  col1         col2          col3*
                             // |------------|-------------|-------------------|
                             //  shell:=       1:           exec:=
                             //  shell:=       1:           input:="my test"
                             //  mod1:         1:           s = `>`<shell>
                             //  mod1:         1:           s.exec
                             //  mod1:         s.exec       inp = s.input
                             // |------------|-------------|-------------------|
                             //                             _<monad_name>

    TOKEN_DOT,               //  col1         col2*          col3*
                             // |------------|-------------|-------------------|
                             //  m1           1:            my_bool
                             //  m1           1:            my_str = "a"
                             //  m2:          m1.my_bool:   `a`<print>.exec
                             //  m2:          1:            a = m1.my_str
                             // |------------|-------------|-------------------|
                             //                (m).(bool)    (_ =) (m).(var)

    TOKEN_AS,                //  col1         col2*          col3
                             // |------------|-------------|-------------------|
                             //  m:            !loop:        i = 2
                             //  m:            !loop:        loop
                             //  m:            loop:         i--
                             //  m:            i as 1:       !loop
                             //  m:            i as 1:       i--
                             //  m:            i as 0:       'done'<print>.exec
                             // |------------|-------------|-------------------|
                             //                as
                             // (for (i=2;i>1;i--) {}; i--; print('done');)

    TOKEN_LOOP,              //  col1         col2*          col3
                             // |------------|-------------|-------------------|
                             //  m:            !loop:        i = 2
                             //  m:            !loop:        loop
                             //  m:            loop:         i--
                             //  m:            i as 1:       !loop
                             //  m:            i as 1:       i--
                             //  m:            i as 0:       'done'<print>.exec
                             // |------------|-------------|-------------------|
                             //                loop          loop
                             //                (!)loop       (!)loop
                             //                (read-only)   (write-only)
                             //
                             // (for (i=2;i>1;i--) {}; i--; print('done');)

    TOKEN_EQUALS,
    // TOKEN_DEFAULT_EQUALS, // := (1st, 3rd columns)
                             // not needed as its own token since the
                             // parser can infer the tokenized sequence {:,=}

    TOKEN_DECREMENT, // -- (3rd column only: var--)
    TOKEN_INCREMENT, // ++ (3rd column only: var++)
    TOKEN_INVALID, // other/unknown
    TOKEN_TEXT, // comment or content inside of a string marker
    TOKEN_EOP // end of program
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
    int column;
    int indent_level; // 0, 1, 2
} Token;

Token *tokenize(const char *source);

#endif
