ext_mod:=
    1:
        a := ``
        b := ``
        res := ``
        !str_cat:=

m1:
    1:
        str = `test`

m2:
    1:
        em = `test`<ext_mod>
    0:
        Note: this (deep reach on rhs) is not supported: 
        em.str_cat = m1.str<ext_mod>.res
        Neither is assigning a module to another module's variable:
        m1.foo = ext_mod
        Nor can booleans be assigned with equals
        m1.foo = ext_mod.str_cat
        Must do 2 lines sep indents:
        \t ext_mod.str_cat \n \t \t m1.foo 
