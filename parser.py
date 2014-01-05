from sys import exit, stdin

import ply.yacc as yacc

from lexer import Lexer
from ast import Node

tokens = Lexer.tokens
start = "function"

precedence = (
    ('left', 'AND', 'OR'),
    ('left', 'PLUS', 'MINUS'),
    ('left', 'MUL', 'DIV'),
    ('left', 'EQ', 'NE', 'GT', 'LT', 'GTE', 'LTE', 'MOD'),
    ('right', 'UMINUS'),
    ('right', 'NOT'),
    )


def p_function(p):
    """function : DEF ID LPAR parameters RPAR LCB statements RCB"""
    p[0] = Node("func", p.lexer.lineno, [p[4], p[7]], p[2])


def p_function_error(p):
    """function : DEF ID LPAR parameters RPAR LCB RCB"""
    print("line %d: syntax error: empty function %r" % (p.lexer.lineno, p[2]))
    exit(1)


def p_parameters(p):
    """parameters : parameters COMMA ID
                  | ID
                  |"""
    if len(p) < 2:
        sub = None
    elif isinstance(p[1], Node) and p[1].type == "params":
        sub = p[1].sub + [p[3]]
    else:
        sub = [p[1]]
    p[0] = Node("params", p.lexer.lineno, sub)


def p_statements(p):
    """statements : statements statement
                  | statement
                  """
    # code block with list of statements
    if p[1].type == "block":
        sub = p[1].sub + [p[2]]
    else:
        sub = [p[1]]
    p[0] = Node("block", p.lexer.lineno, sub)


def p_statement_assign(p):
    """statement : ID ASSIGN expr SC"""
    p[0] = Node("store", p.lexer.lineno, [p[3]], p[1])


def p_statement_assign_dict(p):
    """statement : ID LCB expr RCB ASSIGN expr SC"""
    p[0] = Node("dict-set", p.lexer.lineno, [p[3], p[6]], p[1])


def p_statement_if(p):
    """statement : IF LPAR expr RPAR LCB statements RCB"""
    p[0] = Node("if", p.lexer.lineno, [p[3], p[6]], p[1])


def p_statement_if_else(p):
    """statement : IF LPAR expr RPAR LCB statements RCB \
                   ELSE LCB statements RCB"""
    p[0] = Node("if-else", p.lexer.lineno, [p[3], p[6], p[10]], p[1])


def p_statement_loop(p):
    """statement : LOOP LPAR expr RPAR LCB statements RCB"""
    p[0] = Node("loop", p.lexer.lineno, [p[3], p[6]], p[1])


def p_statement_return(p):
    """statement : RETURN expr SC"""
    p[0] = Node("return", p.lexer.lineno, [p[2]])


def p_statement_function(p):
    """statement : function"""
    p[0] = p[1]


def p_statement_expr_func(p):
    """statement : expr SC"""
    p[0] = p[1]


def p_expr_binop(p):
    """expr : expr PLUS expr
            | expr MINUS expr
            | expr MUL expr
            | expr DIV expr
            | expr EQ expr
            | expr NE expr
            | expr GT expr
            | expr LT expr
            | expr GTE expr
            | expr LTE expr
            | expr AND expr
            | expr OR expr
            | expr MOD expr
            """
    p[0] = Node("binop", p.lexer.lineno, [p[1], p[3]], p[2])


def p_expr_unaop(p):
    """expr : MINUS expr %prec UMINUS
            | NOT expr
            """
    p[0] = Node("unaop", p.lexer.lineno, [p[2]], p[1])


def p_expr_group(p):
    """expr : LPAR expr RPAR"""
    p[0] = p[2]


def p_expr_int(p):
    """expr : INTEGER"""
    p[0] = Node("numeric", p.lexer.lineno, value=int(p[1]))


def p_expr_float(p):
    """expr : FLOAT"""
    p[0] = Node("numeric", p.lexer.lineno, value=float(p[1]))


def p_expr_string(p):
    """expr : STRING"""
    p[0] = Node("string", p.lexer.lineno, value=p[1])


def p_expr_id(p):
    """expr : ID"""
    p[0] = Node("retrieve", p.lexer.lineno, value=p[1])


def p_expr_dict(p):
    """expr : LCB RCB"""
    p[0] = Node("dict", p.lexer.lineno)


def p_expr_dict_get(p):
    """expr : ID LCB expr RCB"""
    p[0] = Node("dict-get", p.lexer.lineno, [p[3]], p[1])


def p_expr_dict_test(p):
    """expr : ID LCB expr RCB Q"""
    p[0] = Node("dict-test", p.lexer.lineno, [p[3]], p[1])


def p_statement_typeof(p):
    """expr : TYPEOF LPAR expr RPAR"""
    p[0] = Node("typeof", p.lexer.lineno, [p[3]])


def p_statement_clone(p):
    """expr : CLONE LPAR ID RPAR"""
    p[0] = Node("clone", p.lexer.lineno, [Node("retieve", p.lexer.lineno, value=p[3])])


def p_expr_func_call(p):
    """expr : ID LPAR expresions RPAR"""
    p[0] = Node("call", p.lexer.lineno, [p[3]], p[1])


def p_statement_println(p):
    """statement : PRINTLN LPAR expresions RPAR SC"""
    p[0] = Node("println", p.lexer.lineno, [p[3]], p[1])


def p_expr_list(p):
    """expresions : expresions COMMA expr
                  | expr
                  |"""
    if len(p) < 2:
        sub = None
    elif p[1].type == "call-params":
        sub = p[1].sub + [p[3]]
    else:
        sub = [p[1]]
    p[0] = Node("call-params", p.lexer.lineno, sub)


def p_error(p):
    if p is None:
        print("EOF")
    else:
        print("line %d: syntax error, unexpected token %r" % (p.lexer.lineno, p.value))
    #exit(1)


def parse(source, debug=True):
    lexer = Lexer()
    lexer.build()
    yacc.yacc(debug=debug)
    return yacc.parse(source, lexer=lexer.lexer)

if __name__ == "__main__":
    data = stdin.read()
    print(parse(data))

    exit(0)

