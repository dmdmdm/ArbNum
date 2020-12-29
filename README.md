# ArbNum
A simple arbitrary precision library and interactive text program

I tired to do arbitrary signed integer precision a simple as possible.  No crazy optimizations.  The code is intended to be readable.

# What it can do:
- add, subtract, multiply, divide, remainder (mod)
- abs, factorial, gcd, isprime, max, min, random, sign, sqrt

# Files
- *arbnum.cpp* and *arbnum.h* perform the math.  You can drop them into another project.
- *bc_tokeniser.cpp* and *bc_tokenizer.h* break the input into text token.
- *bc_calc.cpp* and *bc_calc.h* perform the math on the token using arbnum.
- *bc.cpp* is just the main
