from sys import exit

import ply.lex as lex


class Lexer(object):
    reserved = {
        'def': 'DEF',
        'if': 'IF',
        'else': 'ELSE',
        'loop': 'LOOP',
        'return': 'RETURN',
        'not': 'NOT',
        'and': 'AND',
        'or': 'OR',
        'mod': 'MOD',
        'println': 'PRINTLN',
        'typeof': 'TYPEOF',
    }
    tokens = (
        'FLOAT',
        'INTEGER',
        'STRING',
        'ASSIGN',
        'PLUS',
        'MINUS',
        'MUL',
        'DIV',
        'LPAR',
        'RPAR',

        'NOT',
        'AND',
        'OR',
        'MOD',

        'EQ',
        'NE',
        'GT',
        'LT',
        'GTE',
        'LTE',

        'DEF',
        'ID',
        'IF',
        'ELSE',
        'LOOP',
        'RETURN',

        'PRINTLN',
        'TYPEOF',

        'LCB',
        'RCB',
        'SC',
        'COMMA',
    )

    t_ignore = ' \t'
    t_ignore_COMMENT = r'[#][^\n]*'

    t_ASSIGN = r':='
    t_PLUS = r'\+'
    t_MINUS = r'-'
    t_MUL = r'\*'
    t_DIV = r'/'
    t_LPAR = r'\('
    t_RPAR = r'\)'

    t_EQ = r'='
    t_NE = r'<>'
    t_GT = r'>'
    t_LT = r'<'
    t_GTE = r'>='
    t_LTE = r'<='

    t_LCB = r'{'
    t_RCB = r'}'
    t_SC = r';'
    t_COMMA = r','

    def t_ID(self, t):
        r'[a-zA-Z][a-zA-Z_0-9]*'
        t.type = self.reserved.get(t.value, 'ID')
        return t

    def t_FLOAT(self, t):
        r'\d+\.\d+'
        t.value = float(t.value)
        return t

    def t_INTEGER(self, t):
        r'\d+'
        t.value = int(t.value)
        return t

    def t_STRING(self, t):
        r'\"([^\\"]|(\\.))*\"'
        t.value = str(t.value)
        return t

    def t_newline(self, t):
        r'\n+'
        t.lexer.lineno += len(t.value)

    def t_error(self, t):
        print("line %d: unknown token %r" % (t.lexer.lineno, t.value[0]))
        exit(1)

    def build(self, **kwargs):
        self.lexer = lex.lex(module=self, **kwargs)

    def test(self, data):
        self.lexer.input(data)
        while True:
            token = self.lexer.token()
            if not token:
                break
            print("%r" % token)

if __name__ == "__main__":
    l = Lexer()
    l.build()
    while True:
        try:
            i = input()
        except EOFError:
            break
        l.test(i)

