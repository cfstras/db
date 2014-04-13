
SRCDIR			= src
TESTDIR			= test
BINDIR			= bin
OBJDIR			= build
BINARY			= sort

## Insert path to gtest source here
GTESTDIR		= /usr/src/gtest
GTEST_LIB		= $(OBJDIR)_test/libgtest.a

CXX 			= clang++
DEBUG			= -O0 -DDEBUG
OPTIMIZED		= -O3
CXXFLAGS		= -I$(SRCDIR) -I$(TESTDIR) -g -Weverything -std=c++11 -Wno-c++98-compat -Wno-c++98-compat-pedantic
LDFLAGS			=
TEST_CXXFLAGS	= $(CXXFLAGS) -isystem $(GTESTDIR)/include -pthread
TEST_LDFLAGS	= $(LDFLAGS) $(GTEST_LIB)


all: build-debug

debug: build-debug
	gdb --args $(BINDIR)/$(BINARY)

build-debug: CXXFLAGS += $(DEBUG)
build-debug: build

test: build-test
	$(BINDIR)/test_$(BINARY)

opt: clean do-opt

do-opt: CXXFLAGS += $(OPTIMIZED)
do-opt: build

run: build
	$(BINDIR)/$(BINARY)

happiness:
	@echo Sorry, not implemented yet.

clean:
	\rm -rf $(BINDIR) $(OBJDIR) $(OBJDIR)_test
	\rm -f externalsort-*

dir:
	mkdir -p $(BINDIR)
	cd $(SRCDIR) && find -type d -exec mkdir -p ../$(OBJDIR)/{} \;
	cd $(TESTDIR) && find -type d -exec mkdir -p ../$(OBJDIR)_test/{} \;

.PHONY: datagen
datagen:
	make -C datagenerator silent

SRCS = $(shell cd $(SRCDIR) && find . -type f -name "*.cpp")
TESTSRCS = $(shell cd $(TESTDIR) && find . -type f -name "*.cpp")

OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)
OBJS_TEST = $(filter-out %main.o,$(OBJS)) $(TESTSRCS:%.cpp=$(OBJDIR)_test/%.o)

#TODO opt build
build: dir $(OBJS) $(BINDIR)/$(BINARY)

$(BINDIR)/$(BINARY): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(BINDIR)/$(BINARY)

build-test: CXXFLAGS+= -DSILENT
build-test: dir datagen $(GTEST_LIB) $(OBJS) $(OBJS_TEST) $(BINDIR)/test_$(BINARY)

$(BINDIR)/test_$(BINARY): $(OBJS_TEST) $(GTEST_LIB)
	$(CXX) $(TEST_CXXFLAGS) $(TEST_LDFLAGS) $(OBJS_TEST) $(GTEST_LIB) -o $(BINDIR)/test_$(BINARY)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)_test/%.o: $(TESTDIR)/%.cpp
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

$(GTEST_LIB): $(GTESTDIR)
	$(CXX) $(TEST_CXXFLAGS) -w -c -isystem $(GTESTDIR)/include -I$(GTESTDIR) -pthread -c \
		$(GTESTDIR)/src/gtest-all.cc -o $(OBJDIR)_test/gtest-all.o
	$(AR) -rv $(GTEST_LIB) $(OBJDIR)_test/gtest-all.o
