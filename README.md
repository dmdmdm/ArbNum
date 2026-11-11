# ArbNum
A simple arbitrary precision library and interactive terminal program.

I tired to do arbitrary real (decimal) math as clearly as possible.  No crazy optimizations but somewhat performant.  The code is intended to be readable.

![Demo](arbnum_demo.gif)

# What it can do
- add, subtract, multiply, divide, remainder (mod)
- abs, factorial, gcd, isprime, lcm, max, min, pow, sign, sqrt
- with integers of any length

# Files
- **util.cpp** are some utility functions.
- **unsigned.cpp** implements class Unsigned to perform arbitrary precision unsigned integer math.
- **integer.cpp** implements class Integer to perform arbitrary precision signed integer math.
- **scaled.cpp** implements class Scaled perform arbitrary precision real number (decimal) math.
- **bc_tokeniser.cpp** breaks the input into text tokens.
- **bc_calc.cpp** performs the math on the token using Scaled.
- **bc.cpp** is the main.

# More details
There is a testAll() function which tests nearly everything in about 20 seconds.

There is a class called Unsigned which performs arbitrary precision unsigned integer operations.

Class Integer uses Unsigned to do arbitrary precision signed integer options.

Class Scaled uses Integer to do signed arbitrary precision real (decimal) operations.

All three classes have C++ operator overloading so you can to do: a + b * c where they are Scaled's.
You can construct with string, int, long, double and it has methods toString(), toInt(), toLong(), toDouble() to get values out.
See file example_use_of_scaled.cpp

Compiles on Fedora, Ubuntu, macOS, Windows.
Requires package readline-devel on Fedora.
Requires package libreadline-dev on Ubuntu.
