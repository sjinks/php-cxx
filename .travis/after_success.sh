#!/bin/bash

PREFIX=$(readlink -enq "$(dirname $0)/../")

if [ "$CXX" = "g++" ]; then
	GCOV=gcov
	ARGS=""
else
	GCOV=llvm-cov
	ARGS=gcov
fi

rm -rf   "$PREFIX/.gcov"
mkdir -p "$PREFIX/.gcov"

$GCOV $ARGS -s "$PREFIX" -ablpr -o .build/phpcxx phpcxx/*.cpp
$GCOV $ARGS -s "$PREFIX" -ablpr -o .build/test   test/*.cpp
mv *.gcov "$PREFIX/.gcov"

/bin/bash <(curl -s https://codecov.io/bash) -f "$PREFIX/.gcov/*.gcov" -X gcov
