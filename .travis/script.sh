#!/bin/sh

set -e

if [ "$NOPCH" = "1" ]; then
	make -j$(getconf _NPROCESSORS_ONLN) PHP_CONFIG=/usr/bin/php-config$PHPVER TESTER_LDLIBS=-lphp$PHPVER
	./.lib/php-cxx-test
else
	make -j$(getconf _NPROCESSORS_ONLN) COVERAGE=1 PHP_CONFIG=/usr/bin/php-config$PHPVER TESTER_LDLIBS=-lphp$PHPVER
	./.lib/php-cxx-test --gtest_repeat=2
	valgrind --error-exitcode=1 --fullpath-after=/php-cxx/ --suppressions=php-cxx-test.sup --gen-suppressions=all --leak-check=full --show-leak-kinds=all --track-origins=yes ./.lib/php-cxx-test --gtest_repeat=2
fi
