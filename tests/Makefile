JTC = ../compiler.py

all: operators.jt.out

check: all
	./run_tests.sh

operators.jt.out: operators.jt
	$(JTC) $<

clean:
	rm -f *.out

