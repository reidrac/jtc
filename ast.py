from collections import defaultdict

class Node(object):

    source = "prog"
    funcs = []

    unique = defaultdict(int)

    def __init__(self, type, lineno, sub=None, value=None):
        self.type = type
        self.sub = sub or []
        self.value = value
        self.lineno = lineno
        self.uvalue = None

        if self.type == "func":
            Node.funcs.append(self)
            Node.unique[self.value] += 1
            self.uvalue = "%s_%d" % (self.value, Node.unique[self.value])

    def __repr__(self):
        return u"(%d: %s -> sub: %r value: %r)" % (self.lineno,
                                                   self.type,
                                                   self.sub,
                                                   self.value,
                                                   )
