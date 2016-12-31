PHP_CONFIG      = php-config
SRCDIR          = $(realpath $(dir $(lastword $(filter Makefile,$(MAKEFILE_LIST)))))

CPPFLAGS_EXTRA  = -I$(SRCDIR) $(shell $(PHP_CONFIG) --includes) -DBUILDING_PHPCXX
CXXFLAGS_EXTRA  = -Wall -Wextra -Wno-unused-parameter -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11 -fpic
LDFLAGS_EXTRA   = $(shell $(PHP_CONFIG) --ldflags) -pthread

SHARED_LIBRARY  = .lib/php-cxx.so
STATIC_LIBRARY  = .lib/php-cxx.a
TESTER          = .lib/php-cxx-test

TESTER_LDLIBS   = -lphp7

TARGET          = $(SHARED_LIBRARY) $(STATIC_LIBRARY) $(TESTER)

LIBRARY_CXX_SOURCES = \
	phpcxx/argument.cpp \
	phpcxx/extension.cpp \
	phpcxx/extension_p.cpp \
	phpcxx/function.cpp \
	phpcxx/parameters.cpp \
	phpcxx/phpexception.cpp \
	phpcxx/value.cpp

ifndef CXXFLAGS
CXXFLAGS = -O2 -g
endif

ifeq ($(COVERAGE),1)
CXXFLAGS_EXTRA += -O0 -coverage
LDFLAGS_EXTRA  += -coverage
endif

TESTER_CXX_SOURCES = test/tester.cpp test/testsapi.cpp
TESTER_CXX_OBJS    = $(patsubst %.cpp,.build/%.o,$(TESTER_CXX_SOURCES))
LIBRARY_CXX_OBJS   = $(patsubst %.cpp,.build/%.o,$(LIBRARY_CXX_SOURCES))

CXX_OBJS    = $(LIBRARY_CXX_OBJS) $(TESTER_CXX_OBJS)
OBJS        = $(CXX_OBJS)
DEPS        = $(patsubst %.o,%.d,$(OBJS))
COV_GCDA    = $(patsubst %.o,%.gcda,$(OBJS))
COV_GCNO    = $(patsubst %.o,%.gcno,$(OBJS))

all: $(TARGET)

build_directory: .build .build/phpcxx .build/test
.build .build/phpcxx .build/test:
	mkdir -p "$@"

output_directory: .lib
.lib:
	mkdir -p "$@"

$(TESTER): $(TESTER_CXX_OBJS) $(LIBRARY_CXX_OBJS) | output_directory
	$(CXX) $(LDFLAGS) $(LDFLAGS_EXTRA) $^ $(TESTER_LDLIBS) $(LDLIBS) -o "$@"

$(SHARED_LIBRARY): $(LIBRARY_CXX_OBJS) | output_directory
	$(CXX) $(LDFLAGS) -shared $(LDFLAGS_EXTRA) $^ $(LDLIBS) -o "$@"

$(STATIC_LIBRARY): $(LIBRARY_CXX_OBJS) | output_directory
	$(AR) rcs "$@" $^

.build/%.o: %.cpp | build_directory
	$(CXX) $(CPPFLAGS) $(CPPFLAGS_EXTRA) $(CXXFLAGS) $(CXXFLAGS_EXTRA) -c "$<" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o "$@"

clean-deps:
	-rm -f $(DEPS)

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(DEPS)
endif

clean:
	-rm -f $(TARGET)
	-rm -f $(OBJS)
	-rm -f $(DEPS)
	-rm -f $(COV_GCDA) $(COV_GCNO)

clean-coverage:
	-rm -rf .tracefile coverage $(COV_GCDA)

coverage: clean-coverage
	$(MAKE) $(TESTER) COVERAGE=1 MAKEFLAGS="$(MAKEFLAGS)"
	-$(TESTER)
	lcov -q -d .build -c -b . -o .tracefile
	lcov -q -r .tracefile "/usr/include/*" "$(shell $(PHP_CONFIG) --include-dir)/*" -o .tracefile
	genhtml -q --legend -o coverage -t "phpcxx Code Coverage" .tracefile

.PHONY: build_directory output_directory clean coverage clean-coverage 
