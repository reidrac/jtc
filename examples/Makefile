JTC = ../compiler.py

all: hello.jt.out fib.jt.out prime.jt.out reference.jt.out operators.jt.out

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

clean:
	rm -f *.out

