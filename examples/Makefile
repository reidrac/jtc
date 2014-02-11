JTC = ../compiler.py

all: hello.jt.out fib.jt.out prime.jt.out reference.jt.out operators.jt.out \
	contexts.jt.out fib-memo.jt.out dict-fun.jt.out bstree.jt.out fizzbuzz.jt.out

hello.jt.out: hello.jt
	$(JTC) $<
fib.jt.out: fib.jt
	$(JTC) $<
prime.jt.out: prime.jt
	$(JTC) $<
reference.jt.out: reference.jt
	$(JTC) $<
operators.jt.out: operators.jt
	$(JTC) $<
contexts.jt.out: contexts.jt
	$(JTC) $<
fib-memo.jt.out: fib-memo.jt
	$(JTC) $<
dict-fun.jt.out: dict-fun.jt
	$(JTC) $<
bstree.jt.out: bstree.jt
	$(JTC) $<
fizzbuzz.jt.out: fizzbuzz.jt
	$(JTC) $<

clean:
	rm -f *.out

