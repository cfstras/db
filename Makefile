
SRCDIR			= src
TESTDIR			= test
BINDIR			= bin
OBJDIR			= build
BINARY			= sort
TEST_BINARY		= test_db

## Insert path to gtest source here
GTESTDIR		= /usr/src/gtest
GTEST_LIB		= $(OBJDIR)_test/libgtest.a

CXX 			= clang++
AR				= ar -rv
#AR				= llvm-ar r
DEBUG			= -O0 -DDEBUG
OPTIMIZED		= -O3
CXXFLAGS		= -I$(SRCDIR) -I$(TESTDIR) -g -Wall -Werror -std=c++11 -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-mismatched-tags
OBJFLAGS		= $(CXXFLAGS) -pthread
LDFLAGS			= -pthread
TEST_CXXFLAGS	= $(CXXFLAGS) -isystem $(GTESTDIR)/include
TEST_OBJFLAGS	= $(TEST_CXXFLAGS) -pthread

TEST_LDFLAGS	= $(LDFLAGS) $(GTEST_LIB)


all: build-test

debug: build-debug
#	gdb --args $(BINDIR)/$(BINARY)

cov:
	kcov --include-path=$(shell pwd)/src/ report/ bin/test_db; true
	gnome-open report/index.html

build-debug: CXXFLAGS += $(DEBUG)
build-debug:
	[ -f $(OBJDIR)/OPTIMIZED ] && $(MAKE) clean; true
	[ -f $(OBJDIR)/DEBUG ] || $(MAKE) clean; true
	$(MAKE) dir
	touch $(OBJDIR)/DEBUG
	$(MAKE) build-test

.PHONY: test
test:
	[ -f $(OBJDIR)/OPTIMIZED ] && $(MAKE) clean; true
	[ -f $(OBJDIR)/DEBUG ] && $(MAKE) clean; true
	$(MAKE) build-test-flag
	rm -rf test_data/
	$(BINDIR)/$(TEST_BINARY)

.PHONY: opt
opt:
	[ -f $(OBJDIR)/OPTIMIZED ] || $(MAKE) clean; true
	[ -f $(OBJDIR)/DEBUG ] && $(MAKE) clean; true
	$(MAKE) dir
	touch $(OBJDIR)/OPTIMIZED
	$(MAKE) build-opt

build-opt: CXXFLAGS += $(OPTIMIZED)
build-opt: build

run: build
	$(BINDIR)/$(BINARY)

happiness:
	@echo Sorry, not implemented yet.

clean:
	\rm -rf $(BINDIR) $(OBJDIR) $(OBJDIR)_test
	\rm -f externalsort-*

dir:
	@mkdir -p $(BINDIR) $(OBJDIR) $(OBJDIR)_test
	@cd $(SRCDIR) && find * -type d -exec mkdir -p ../$(OBJDIR)/{} \;
	@cd $(TESTDIR) && find * -type d -exec mkdir -p ../$(OBJDIR)_test/{} \;

.PHONY: datagen
datagen:
	$(MAKE) -C datagenerator silent

SRCS = $(shell cd $(SRCDIR) && find * -type f -name "*.cpp")

TESTSRCS = $(shell cd $(TESTDIR) && find * -type f -name "*.cpp")

OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)
OBJS_TEST = $(filter-out %main.o,$(OBJS)) $(TESTSRCS:%.cpp=$(OBJDIR)_test/%.o)

#TODO opt build
build: dir $(OBJS) $(BINDIR)/$(BINARY)

$(BINDIR)/$(BINARY): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(BINDIR)/$(BINARY)

build-test-flag: CXXFLAGS+= -DSILENT -O2
build-test-flag: build-test

build-test: dir datagen $(GTEST_LIB) $(OBJS) $(OBJS_TEST) $(BINDIR)/$(TEST_BINARY) $(BINDIR)/$(BINARY)

$(BINDIR)/$(TEST_BINARY): $(OBJS_TEST) $(GTEST_LIB)
	$(CXX) $(TEST_CXXFLAGS) $(TEST_LDFLAGS) $(OBJS_TEST) $(GTEST_LIB) -o $(BINDIR)/$(TEST_BINARY)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(OBJFLAGS) -c $< -MMD -o $@

$(OBJDIR)_test/%.o: $(TESTDIR)/%.cpp
	$(CXX) $(TEST_OBJFLAGS) -c -MMD $< -o $@

-include $(OBJDIR)/*.d
-include $(OBJDIR)_test/*.d

$(GTEST_LIB): $(GTESTDIR)
	$(CXX) $(TEST_CXXFLAGS) -w -c -I$(GTESTDIR) -c \
		$(GTESTDIR)/src/gtest-all.cc -o $(OBJDIR)_test/gtest-all.o
	$(AR) $(GTEST_LIB) $(OBJDIR)_test/gtest-all.o
