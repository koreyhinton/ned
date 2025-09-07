# Ned language in 2 minutes* or less

Note: this is a (conceptual) work in progress.

There are 3 indent levels. Think of it as a text editor optimized version of a 3-column spreadsheet:

`module specifier | run bit scan | execution line`

Nums are assigned on the execution line (last indent). Bools are also assigned on the execution line `!loop`, `loop` (false, true)

Only booleans can be read on the run bit scan (mid-indent)

```
print:=
    1:
        exec:=

a_control_module:
    0:
         mid-indent is a run bit check and since 0 won't run, this is a comment.
         Notes: loop is a builtin (mutable in last indent, checked in mid-indent)
         booleans are assigned just by referencing them in the last indent position (my_boolean to set it to 1, !my_boolean to set it to 0)
         booleans (or number cast or special 0 or 1 special global boolean) are checked in the middle indent position
         : is a no-op
    !loop:
          loop
          i = 3
    loop:
          i--
    i as 1:
        !loop

other_module:
    1:
        :

a_control_module:
last_module:
    0:
        First Note: other_module created a break, and so a_control_module specified before last_module (in first col.) actually ran it again
        Second Note: upon module re-run it kept its own variable values as they were after the firs run (a_control_module.i == 1 and !loop)
        Lastly: since we are in a new module, the module is treated as a data module, and its variables can be accessed:
    1:
        a_control_module.i<print>.exec
    0:
        ^^^ prints out a 1
```

## External modules

External modules are declared with the default implementation `:=` operator:

```
ext_mod:=
```

In order to invoke their special bool functions, you create them with by flagging them with a non-bool primitive type cast: `{num|str}<ext_mod>.special_bool`.

External modules do not have normal booleans and should fail to compile a falsy assignment:

```
external_mod:=
    1:
        special_bool:=
normal_mod:
    1:
        em = 1<external_mod>
        !em.special_bool
    0:
        ^^^ falsy should fail compilation
        vvv truthy will compile (as it invokes a special_bool C lib function)
    1:
        em.special_bool
    em.special_bool:
        :
    0:
        ^^^ enters code block conditioned on external special_bool status
```

Dictionary **k**ey/**v**alue example:

```
int_str_dict:=
    0:
        vars
    1:
        temp_k := 0
        temp_v := ``
        lookup_k := 0
        lookup_v := ``
    0:
        special bool functions (can be falsy in default implementation)
    1:
        !add_temp:=
        !lookup:=

program_module:
    0:
        0 initial capacity
    1:
        d = 0<int_str_dict>
        d.temp_k = 1
        d.temp_v = `one`
        d.add_temp
        d.lookup_key = 1
        d.lookup
    d.lookup:
        val = d.lookup_v
```



# Key Terms

- boolean - a builtin constant of `0` or `1` (or cast to it), or a builtin mutable `loop`, or your own specified variable.
- default implementation - a program version with test inputs (must be specified for external modules) to use dummy values.
- module - is it the external C library code kind you compiled in with your ned program (_external module_)?
  - or is it the control flow kind (_control module_)?
    - or maybe you are referencing the control module to re-run (first col. ref. w / module break above and below it)
      - or, now that the module lost its power, it is simply a _data module_
- ned - a language existing mostly in my head
- nedry - hypothetical compiler
- number - at least support integers that can count down and cast to become a 1 boolean for looping behavior
- operator - `=` _assignment operator_. at least support the `--` decrement operator to loop. `:` in last indent is a no-op. might allow it to be by itself an implied 1: in the mid-col.

# Demo

In theory, nedry (Dennis' compiler) would also compile the more verbose and fully specified, spreadsheet-compatible language version.

Nedry never did check in his compiler, but found footage suggests he wrote his code by clumsily using a mouse inside a spreadsheet application:

https://github.com/user-attachments/assets/38dc68dd-fe78-4f71-a04e-45e44047e9d8

([found footage](access.mov). \[[1989](https://github.com/koreyhinton/ned/commit/6629ca96928220c64bd39279974d849b3cb7cc38)\])
