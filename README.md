# ArbNum
A simple arbitrary precision library and interactive text program

I tired to do arbitrary signed integer precision a simple as possible.  No crazy optimizations but somewhat performant.  The code is intended to be readable.

# What it can do
- add, subtract, multiply, divide, remainder (mod)
- abs, factorial, gcd, isprime, max, min, random, sign, sqrt

# Files
- *arbnum.cpp* and *arbnum.h* perform the math.  You can drop them into another project.
- *bc_tokeniser.cpp* and *bc_tokenizer.h* break the input into text token.
- *bc_calc.cpp* and *bc_calc.h* perform the math on the token using arbnum.
- *bc.cpp* is just the main

# More details
There is a testAll() function which tests nearly everything.  Takes about 10 seconds to run.

There is a class called Unsigned which performs arbitrary precison unsigned integer operations.

Class ArbNum uses Unsigned to do signed arbitrary precison unsigned integer operations.
For example: it turns 10 + (-20) into 20 - 10 and calls Unsigned.subtract(20, 10)
