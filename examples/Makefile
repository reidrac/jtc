JTC = ../compiler.py

all: fib.jt.out prime.jt.out reference.jt.out operators.jt.out

fib.jt.out: fib.jt
	$(JTC) $< --verbose
prime.jt.out: prime.jt
	$(JTC) $< --verbose
reference.jt.out: reference.jt
	$(JTC) $< --verbose
operators.jt.out: operators.jt
	$(JTC) $< --verbose

clean:
	rm -f *.out

