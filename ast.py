class Node(object):

    source = "prog"
    funcs = []

    def __init__(self, type, lineno, sub=None, value=None):
        self.type = type
        self.sub = sub or []
        self.value = value
        self.lineno = lineno

        if self.type == "func":
            Node.funcs.append(self)

    def __repr__(self):
        return u"(%d: %s -> sub: %r value: %r)" % (self.lineno,
                                                   self.type,
                                                   self.sub,
                                                   self.value,
                                                   )
