PHP_CONFIG          = php-config
SRCDIR              = $(realpath $(dir $(lastword $(filter Makefile,$(MAKEFILE_LIST)))))

CPPFLAGS_EXTRA      = $(shell $(PHP_CONFIG) --includes)
CXXFLAGS_EXTRA      = -Wall -Wextra -Wno-unused-parameter -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11 -fPIC
LDFLAGS_EXTRA       = -pthread

SHARED_LIBRARY      = .lib/php-cxx.so
STATIC_LIBRARY      = .lib/php-cxx.a
TESTER              = .lib/php-cxx-test

PHPCXX_CPPFLAGS     = -DBUILDING_PHPCXX -DPHPCXX_DEBUG
PHPCXX_CPPFLAGS_PCH = -include .build/phpcxx/precompiled.h -Winvalid-pch
PHPCXX_LDFLAGS      = -shared $(shell $(PHP_CONFIG) --ldflags)

TESTER_CPPFLAGS     = -I$(SRCDIR) -DPHPCXX_DEBUG
TESTER_CPPFLAGS_PCH = -include .build/test/precompiled.h -Winvalid-pch
TESTER_LDFLAGS      = 
TESTER_LDLIBS       = -lphp7

TARGET              = $(SHARED_LIBRARY) $(STATIC_LIBRARY) $(TESTER)

LIBRARY_CXX_SOURCES = \
	phpcxx/argument.cpp \
	phpcxx/array.cpp \
	phpcxx/constant.cpp \
	phpcxx/callable.cpp \
	phpcxx/class_p.cpp \
	phpcxx/classbase.cpp \
	phpcxx/classconstant.cpp \
	phpcxx/exception.cpp \
	phpcxx/fcall.cpp \
	phpcxx/function.cpp \
	phpcxx/method.cpp \
	phpcxx/module.cpp \
	phpcxx/module_p.cpp \
	phpcxx/object.cpp \
	phpcxx/objectdimension.cpp \
	phpcxx/operators.cpp \
	phpcxx/parameters.cpp \
	phpcxx/phpexception.cpp \
	phpcxx/superglobal.cpp \
	phpcxx/value.cpp \
	phpcxx/variables.cpp
	
LIBRARY_GCH = .build/phpcxx/precompiled.h.gch
LIBRARY_PCH = phpcxx/precompiled.h

TESTER_CXX_SOURCES = \
	test/tester.cpp \
	test/testsapi.cpp \
	test/test_arguments.cpp \
	test/test_lifecycle.cpp \
	test/test_moduleglobals.cpp \
	test/test_constants.cpp \
	test/test_value.cpp \
	test/test_arrays.cpp \
	test/test_superglobals.cpp \
	test/test_parameters.cpp \
	test/test_functions.cpp \
	test/test_zendstring.cpp \
	test/test_callable.cpp \
	test/test_fcall.cpp \
	test/test_phpexception.cpp \
	test/test_hotp.cpp

TESTER_GCH = .build/test/precompiled.h.gch
TESTER_PCH = test/precompiled.h

### Google Test
GTEST_DIR  = /usr/src/gtest
GTEST_SRCS = $(wildcard $(GTEST_DIR)/src/*.cc)
GTEST_HDRS = $(wildcard /usr/include/gtest/*.h) $(wildcard /usr/include/gtest/internal/*.h)
### END Google Test

ifndef CXXFLAGS
CXXFLAGS = -O0 -g3
ifeq ($(CXX),g++)
CPPFLAGS = -D_GLIBCXX_PROFILE
endif
endif

ifeq ($(CXX),clang++)
CPPFLAGS_EXTRA = $(subst -I,-isystem ,$(shell $(PHP_CONFIG) --includes))
# CXXFLAGS_EXTRA += -Weverything -Werror -Wno-c++98-compat -Wno-documentation -Wno-documentation-unknown-command -Wno-switch-enum -Wno-error=padded -Wno-error=exit-time-destructors -Wno-error=float-equal -Wno-error=global-constructors -Wno-error=double-promotion -Wno-error=weak-vtables -Wno-shadow
endif

ifeq ($(COVERAGE),1)
CXXFLAGS_EXTRA += -O0 -coverage
LDFLAGS_EXTRA  += -coverage
endif

ifeq ($(NOPCH),1)
PHPCXX_CPPFLAGS_PCH = 
TESTER_CPPFLAGS_PCH = 
endif

TESTER_CXX_OBJS    = $(patsubst %.cpp,.build/%.o,$(TESTER_CXX_SOURCES))
LIBRARY_CXX_OBJS   = $(patsubst %.cpp,.build/%.o,$(LIBRARY_CXX_SOURCES))

CXX_OBJS    = $(LIBRARY_CXX_OBJS) $(TESTER_CXX_OBJS)
OBJS        = $(CXX_OBJS)
DEPS        = $(patsubst %.o,%.d,$(OBJS)) $(patsubst %.gch,%.d,$(TESTER_GCH)) $(patsubst %.gch,%.d,$(LIBRARY_GCH))
COV_GCDA    = $(patsubst %.o,%.gcda,$(OBJS))
COV_GCNO    = $(patsubst %.o,%.gcno,$(OBJS))

all: $(TARGET)

build_directory: .build .build/phpcxx .build/test
.build .build/phpcxx .build/test:
	mkdir -p "$@"

output_directory: .lib
.lib:
	mkdir -p "$@"

ifneq ($(NOPCH),1)
$(LIBRARY_GCH): $(LIBRARY_PCH) | build_directory
	$(CXX) $(CPPFLAGS) $(PHPCXX_CPPFLAGS) $(CPPFLAGS_EXTRA) $(CXXFLAGS) $(CXXFLAGS_EXTRA) -MMD -MP -MF"$(@:%.gch=%.d)" -MT"$(@:%.gch=%.d)" -MT"$@" -o "$@" "$<"

$(TESTER_GCH): $(TESTER_PCH) | build_directory
	$(CXX) $(CPPFLAGS) $(TESTER_CPPFLAGS) $(CPPFLAGS_EXTRA) $(CXXFLAGS) $(CXXFLAGS_EXTRA) -MMD -MP -MF"$(@:%.gch=%.d)" -MT"$(@:%.gch=%.d)" -MT"$@" -o "$@" "$<"

$(LIBRARY_CXX_OBJS): $(LIBRARY_GCH)
$(TESTER_CXX_OBJS): $(TESTER_GCH)
else
$(OBJS): | clean-pch
endif

$(TESTER): $(TESTER_CXX_OBJS) $(STATIC_LIBRARY) .build/gtest-all.o | output_directory
	$(CXX) $(LDFLAGS) $(TESTER_LDFLAGS) $(LDFLAGS_EXTRA) $^ $(TESTER_LDLIBS) $(LDLIBS) -o "$@"

$(SHARED_LIBRARY): $(LIBRARY_CXX_OBJS) | output_directory
	$(CXX) $(LDFLAGS) $(PHPCXX_LDFLAGS) $(LDFLAGS_EXTRA) $^ $(LDLIBS) -o "$@"

$(STATIC_LIBRARY): $(LIBRARY_CXX_OBJS) | output_directory
	$(AR) rcs "$@" $^

ifeq ($(CXX),clang++)
.build/gtest-all.o: $(GTEST_SRCS) | output_directory
	$(CXX) -isystem $(GTEST_DIR) $(CXXFLAGS) -g -O2 -c $(GTEST_DIR)/src/gtest-all.cc -o "$@"
else
.build/gtest-all.o: $(GTEST_SRCS) | output_directory
	$(CXX) -I$(GTEST_DIR) $(CXXFLAGS) -g -O2 -c $(GTEST_DIR)/src/gtest-all.cc -o "$@"
endif

.build/phpcxx/%.o: phpcxx/%.cpp | build_directory
	$(CXX) $(CPPFLAGS) $(PHPCXX_CPPFLAGS) $(PHPCXX_CPPFLAGS_PCH) $(CPPFLAGS_EXTRA) $(CXXFLAGS) $(CXXFLAGS_EXTRA) -c "$<" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o "$@"

.build/test/%.o: test/%.cpp | build_directory
	$(CXX) $(CPPFLAGS) $(TESTER_CPPFLAGS) $(TESTER_CPPFLAGS_PCH) $(CPPFLAGS_EXTRA) $(CXXFLAGS) $(CXXFLAGS_EXTRA) -c "$<" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o "$@"

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
	-rm -f $(LIBRARY_GCH) $(TESTER_GCH)

clean-coverage:
	-rm -rf .tracefile coverage $(COV_GCDA)

clean-pch:
	-rm -f $(TESTER_GCH) $(LIBRARY_GCH)

test: $(TESTER)
	$(TESTER)

coverage: clean-coverage
	$(MAKE) $(TESTER) COVERAGE=1 MAKEFLAGS="$(MAKEFLAGS)"
	-$(TESTER)
	lcov -q -d .build -c -b . -o .tracefile
	lcov -q -r .tracefile "/usr/include/*" "$(shell $(PHP_CONFIG) --include-dir)/*" -o .tracefile
	genhtml -q --legend -o coverage -t "phpcxx Code Coverage" .tracefile

.PHONY: build_directory output_directory clean coverage clean-coverage clean-pch 
