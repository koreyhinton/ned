# DFA

Possibly the most complex lexical path in the dfa is this one. See [ned dfa](docs/ned_dfa.svg):

```
\t \t my_module.var_a = 1<print>.length
```

Note that ned won't support nesting further, so these examples below should not compile:

```
\t \t my_module.other_module.var_a = 1<print>.length
\t \t my_module.var_a = 1<print>.length<print>.length
```

Line 1 above is impossible because ned modules would not be allowed to contain other modules as variable references.

Line 2 above can be supported though if re-written as separate lines:

```
print:=
\t 1:
\t \t length:=1
\t \t exec:=

my_module:
\t 1:
\t \t var_a = 0

run_module:
\t 1:
\t \t len = 1<print>.length
\t \t my_module.var_a = len<print>.length
```
