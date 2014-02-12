#!/bin/bash

TESTS="*.jt.out"
FAILED=""

for test in $TESTS; do
	./$test
	if [ $? -ne 0 ]; then
		FAILED="$FAILED  $test\n"
	fi
done

if [ "X$FAILED" != "X" ]; then
    echo -e "\n*** Some tests FAILED:\n\n$FAILED\n"
	exit 1
fi

echo "*** All tests PASSED"
exit 0

