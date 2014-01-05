#!/usr/bin/env python

from sys import exit, stdin
from os import environ, path, unlink
from tempfile import NamedTemporaryFile
from subprocess import call
from argparse import ArgumentParser

import parser
import lexer

__author__ = "Juan J. Martinez <jjm@usebox.net>"
__version__ = "0.2"
app_name = "JTC"
project_url = "https://github.com/reidrac/jtc/"

operators = ('+', '-', '*', '/', '=', '<>', '>', '<', '>=', '<=', 'and', 'or', 'mod', 'not')
enum = ('ADD', 'SUB', 'MUL', 'DIV', 'EQ', 'NE', 'GT', 'LT', 'GE', 'LE', 'AND', 'OR', 'MOD', 'NOT')
op_trans = dict(zip(operators, enum))


class Id(object):
    index = 1
    ids = dict()
    stack = []

    #INTEGER = 1
    #FLOAT = 2
    #STRING = 3
    ID = 10
    FUNC = 11

    @staticmethod
    def add(lineno, type, id):
        Id.ids[id] = Id(lineno, type)
        Id.index += 1
        return Id.ids[id].index

    @staticmethod
    def get(id):
        return Id.ids[id]

    @staticmethod
    def enter():
        Id.stack.append([Id.ids, Id.index])
        Id.ids = dict()
        Id.index = 1

    @staticmethod
    def leave():
        Id.ids, Id.index = Id.stack[-1]
        Id.stack = Id.stack[:-1]

    @staticmethod
    def exists(id):
        try:
            return Id.ids[id]
        except KeyError:
            return None

    def __init__(self, lineno, type):
        self.index = Id.index
        self.lineno = lineno
        self.type = type

    def __repr__(self):
        return u"%r (%r, %r)" % (self.index, self.lineno, self.type)


def func_sign(node):
    params = node.sub[0].sub
    cparams = ', '.join(["obj *%s" % p for p in params])
    return """\
obj *_%s(%s)\
""" % (node.value, cparams)


def do_func(node):
    Id.enter()
    # make the function available inside itself to support
    # recursive calls
    Id.add(node.lineno, Id.FUNC, node.value)
    output = "\n" + func_sign(node) + " { st *_ctx = NULL; "
    for value in node.sub[0].sub:
        index = Id.add(node.sub[0].lineno, Id.ID, value)
        output += "store(&_ctx, %d, %d, %s); " % (node.sub[0].lineno, index, value)
    output += " %s" % do_block(node.sub[1])

    # all functions return 0 by default (unless there's an user provided return!)
    output += "\nreturn o_return(&_ctx, o_int(0, 0)); }\n"

    Id.leave()
    return output


def do_if(node):
    expr, block = node.sub
    return """
if (o_lval(%d, %s)) { %s}
""" % (node.lineno, do_expr(expr), do_block(block))


def do_if_else(node):
    expr, block, elseb = node.sub
    return """
if (o_lval(%d, %s)) { %s} else { %s}
""" % (node.lineno, do_expr(expr), do_block(block), do_block(elseb))


def do_loop(node):
    expr, block = node.sub
    return """
while (o_lval(%d, %s)) { %s}
""" % (node.lineno, do_expr(expr), do_block(block))


def do_retrieve(node):
    if not Id.exists(node.value):
        print("line %d: undefined identifier %r" % (node.lineno, node.value))
        exit(1)
    index = Id.get(node.value).index
    return "retrieve(&_ctx, %d, %d)" % (node.lineno, index)


def do_expr(node):
    output = ""
    if node.type == "retrieve":
        output += do_retrieve(node)
    elif node.type == "numeric":
        if isinstance(node.value, int):
            output += "o_int(%d, %d)" % (node.lineno, node.value)
        else:
            output += "o_float(%d, %d)" % (node.lineno, node.value)
    elif node.type == "string":
        output += "o_string(%d, %s)" % (node.lineno, node.value)
    elif node.type == "binop":
        output += "o_op(%d, %s, %s, %s)" % (node.lineno, op_trans[node.value], do_expr(node.sub[0]), do_expr(node.sub[1]))
    elif node.type == "unaop":
        output += "o_op(%d, %s, %s, NULL)" % (node.lineno, op_trans[node.value], do_expr(node.sub[0]))
    elif node.type == "call":
        exists = Id.exists(node.value)
        if exists and exists.type == Id.FUNC:
            params = ', '.join([do_expr(p) for p in node.sub[0].sub])
            output += "_%s(%s)" % (node.value, params)
        else:
            print("line %d: undefined function %r" % (node.lineno, node.value))
            exit(1)
    elif node.type == "typeof":
        output += "o_typeof(%d, %s)" % (node.lineno, do_expr(node.sub[0]))
    elif node.type == "clone":
        output += "o_clone(%d, %s)" % (node.lineno, do_retrieve(node.sub[0]))

    return output


def do_block(node):
    output = ""
    for c in node.sub:
        if c.type == "func":
            # make the function available to this scope
            Id.add(c.lineno, Id.FUNC, c.value)
        elif c.type == "store":
            exists = Id.exists(c.value)
            if exists and exists.type != Id.ID:
                print("line %d: %r already defined as function in line %d" % (c.lineno, c.value, exists.lineno))
            if not exists:
                index = Id.add(c.lineno, Id.ID, c.value)
            else:
                index = Id.get(c.value).index
            output += "store(&_ctx, %d, %d, %s); " % (c.lineno, index, do_expr(c.sub[0]))
        elif c.type == "if":
            output += do_if(c)
        elif c.type == "if-else":
            output += do_if_else(c)
        elif c.type == "loop":
            output += do_loop(c)
        elif c.type == "return":
            output += "return o_return(&_ctx, %s); " % do_expr(c.sub[0])
            # we need the context!
            Id.no_func = True
        elif c.type == "println":
            params = ', '.join([do_expr(p) for p in c.sub[0].sub])
            output += "println(%d, %s); " % (len(c.sub[0].sub), params)
        else:
            output += do_expr(c) + "; "
    return output


def generate(ast):
    output = """\
/*
 * jtc ver %s
 * source: %s
 */
#include "rt.h"

""" % (__version__, ast.source)

    if ast.sub[0].sub:
        print("line %d: syntax error: main function parameters" % ast.sub[0].lineno)
        exit(1)

    for f in ast.funcs:
        output += func_sign(f) + ";\n"

    for f in ast.funcs:
        output += do_func(f)

    output += """
int _ep() { obj *o = _%s(); return o_lval(0, o); }
/* EOF */
""" % ast.value

    return output


if __name__ == "__main__":

    ap = ArgumentParser(description="%s (Juan's Toy Compiler)" % app_name,
                        epilog=project_url,
                        )

    ap.add_argument("source", help="source file to compile (use - for stdin)")
    ap.add_argument("--lexer", action="store_true", help="dump lexer output and exit")
    ap.add_argument("--parser", action="store_true", help="dump parser output and exit")
    ap.add_argument("-c", action="store_true", help="dump C output and exit")
    ap.add_argument("--debug", action="store_true", help="enable debug")
    ap.add_argument("--verbose", action="store_true", help="enable verbose output")
    ap.add_argument("--version", action="version", version="%(prog)s " + __version__)

    args = ap.parse_args()

    if args.verbose:
        print("starting: %s ver %s" % (app_name, __version__))

    if args.verbose:
        print("reading source from:", args.source)

    if args.source == "-":
        source = "<stdin>"
        data = stdin.read()

    else:
        source = args.source
        try:
            fd = open(args.source, "rt")
        except IOError:
            ap.error("failed to open %r" % args.source)

        try:
            data = fd.read()
        except IOError as ex:
            ap.error("failed to read %r: %s" % (args.source, ex))
        finally:
            fd.close()

    if args.lexer:
        l = lexer.Lexer()
        l.build()
        print(l.test(data))
        exit(0)

    ast = parser.parse(data)
    if not ast:
        exit(1)
    ast.source = source

    if args.parser:
        print(ast)
        exit(0)

    if args.verbose:
        print("generating code: %d function(s)" % len(ast.funcs))

    c = generate(ast)
    if args.c:
        print(c)
        exit(1)

    cc = environ.get("CC", "gcc")
    home = environ.get("JTCHOME", path.abspath(path.dirname(__file__)))
    fd = NamedTemporaryFile(mode="wt", suffix=".c", delete=False)
    try:
        fd.write(c)
        fd.close()
        cmd = [cc, "-Wall", "-I%s" % path.join(home, "include"), fd.name, "-o", source + ".out"]
        if args.debug:
            cmd.append("-ggdb")
        else:
            cmd.extend(["-s", "-O2"])
        if args.verbose:
            print("compiling:", ' '.join(cmd))
        try:
            if call(cmd) != 0:
                print("compiled with C errors")
                exit(1)
        except BaseException as ex:
            print("error running the C compiler: %s" % ex)
            exit(1)
    finally:
        unlink(fd.name)

    if args.verbose:
        print("done")

    exit(0)

