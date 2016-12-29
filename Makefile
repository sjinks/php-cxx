PHP_CONFIG     = php-config

CPPFLAGS_EXTRA = $(shell $(PHP_CONFIG) --includes) -DBUILDING_PHPCXX
CXXFLAGS_EXTRA = -Wall -Wextra -Wno-unused-parameter -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11 -fpic -g3
LDFLAGS_EXTRA  = $(shell $(PHP_CONFIG) --ldflags) -pthread

SHARED_LIBRARY = .lib/php-cxx.so
STATIC_LIBRARY = .lib/php-cxx.a
TESTER         = .lib/php-cxx-test

TESTER_LDLIBS  = -lphp7

TARGET         = $(SHARED_LIBRARY) $(STATIC_LIBRARY) $(TESTER)

LIBRARY_CXX_SOURCES = \
	argument.cpp \
	extension.cpp \
	extension_p.cpp \
	function.cpp \
	parameters.cpp \
	phpexception.cpp \
	value.cpp

TESTER_CXX_SOURCES = tester.cpp testsapi.cpp
TESTER_CXX_OBJS    = $(patsubst %.cpp,.build/%.o,$(TESTER_CXX_SOURCES))
LIBRARY_CXX_OBJS   = $(patsubst %.cpp,.build/%.o,$(LIBRARY_CXX_SOURCES))

CXX_OBJS    = $(LIBRARY_CXX_OBJS) $(TESTER_CXX_OBJS)
OBJS        = $(CXX_OBJS)
DEPS        = $(patsubst %.o,%.d,$(OBJS))
COV_GCDA    = $(patsubst %.o,%.gcda,$(OBJS))
COV_GCNO    = $(patsubst %.o,%.gcno,$(OBJS))

all: $(TARGET)

build_directory: .build
.build:
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

.PHONY: build_directory output_directory clean 
