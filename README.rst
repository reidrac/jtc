JTC (Juan's Toy Compiler)
=========================

Years ago I wrote an interpreter for my own programming language, using Flex
and Bison, and it was a fun and rewarding experience. Unfortunately it was the
DOS era and I lost almost everything I wrote back then.

So I've done it again, just for fun. I'm using Python this time and the amazing
PLY (Python Lex-Yacc) to generate the lexer and the parser. Instead of yet another
interpreter I wrote a compiler that generates C code.

I don't think this is the best way of doing things, no optimizations whatsoever,
I don't care too much, etc; but here you are!

How does the language look like? This is how you calculate the 10th Fibonacci
number using **jt**::

  def main() {

    def fib(n) {
      if(n <= 1) {
        return n;
      }

      return fib(n-1) + fib(n-2);
    }

    println(fib(10));
    return 0;
  }

Take a look to the **examples** directory for more programs.


Requirements
------------

The compiler requires:

- Python 3 (3.3.2 used).
- PLY (3.4 used).
- uthash (included in the package).
- A C compiler (by default **gcc**, you can specify a different one with the `CC` env variable).

You can use test the compiler with::

  ./compiler.py examples.fib.jt
  ./examples/fib.jt.out

It should print `55`.


About the language (jt)
-----------------------

It's a toy language and I don't think it's useful for anything, specially because
I bet the compiled programs will leak memory (although it might not).

The program entry point is a function, defined with **def**::

  def main() {

  }

Any function name can be used, although **main** (as in *main function*) sounds OK. The
main function can't have parameters (no CLI interface, sorry).

Other functions can be defined inside any function and any variable identifier will be
local to that function. For example::

  def main() {
    a := 1;
    b := "world";
    c := "whatever",

    def say_hello(a) {
        b := "hello ";

        # c doesn't exist!
        return b + a;
    }

    println(say_hello(b));
  }

This program will print "hello world".

Dynamic typing is used, with these valid types: integer, float and strings. **typeof** can
be used to inspect the type of a variable::

  def types() {
    a := 1;
    println(a, " is ", typeof(a));
    a := 1.0;
    println(a, " is ", typeof(a));
    a := "foo";
    println(a, " is ", typeof(a));
  }
  # will print:
  # 1 is <integer>
  # 1.000000 is <float>
  # foo is <string>

Although some checks are performed at compilation time, the run-time will report
any problem in execution time (specially related to the dynamic types). For example::

  def main() {
    a := "foo";

    # this operation is unsupported and will raise an error
    return a * a;
  }
  # RT_ERROR: line 6: unsuppored type for '*'

Errors can't be caught, remember this is a toy language!

Supported statements in the language are:

:return *expression*:
    Exit the function and return the expression to the caller. In the main function
    a numeric value is expected.

:if (*expression*) { *statements* } [else { *statements* }]:
    If the expression evaluates to true (anything different from zero), the block
    of statements delimited by the curly braces is executed. If the **else** part
    is present, the second block is executed when the expression evaluates to
    false (zero).

:loop (*expression*) { *statements* }:
    The statements delimited by the curly braces are executed repeatedly while
    the expression evaluates to true.

:println(*expression[, expression]*):
    Display on standard output the comma separated list of expressions, followed by
    an end of line.

:typeof(*identifier*):
    Returns a string with the type of the variable. It can be "<integer>" for integers,
    "<float>" for floats and "<string>" for strings.


Operators
---------

The language supports the following operators:

- Assignation `:=`
- Arithmetic operators `+`, `-`, `*`, `/`, `mod`
- Logic operators `=`, `<>`, `>`, `<`, `>=`, `<=`, 'and', 'or', 'not'

Some type conversions are supported depending on the first operand::

  def main() {
    a := "foo";

    println(a + 10);
    println(a + 10 + a);
    println(10 + a + 10 + a);
  }
  # foo10
  # foo10foo
  # RT_ERROR: line 7: unsuported conversion

So basically you can convert between integer and float, and to strings.


License
-------

Copyright (C) 2014 Juan J. Martinez <jjm@usebox.net>

This is free software under the terms of the MIT license (check LICENSE file
for further details).

