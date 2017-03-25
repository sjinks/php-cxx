#!/bin/sh

set -e

make -j$(getconf _NPROCESSORS_ONLN) COVERAGE=1 PHP_CONFIG=/usr/bin/php-config$PHPVER TESTER_LDLIBS=-lphp$PHPVER
valgrind ./.lib/php-cxx-test
