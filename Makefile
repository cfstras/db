
CXX 			= clang++
DEBUG			= -g -O0
OPTIMIZED		= -O3
CXXFLAGS		= -Weverything -std=c++11 -Wno-c++98-compat
LDFLAGS			=
TEST_LDFLAGS	= $(LDFLAGS) -lgtest

SRCDIR			= src
TESTDIR			= test
BINDIR			= bin
OBJDIR			= build
BINARY			= db

default: dir build

#TODO opt build
debug: dir build

test: dir build-test run-tests

happiness:
	@echo Sorry, not implemented yet.

clean:
	\rm -rf $(BINDIR) $(OBJDIR) $(OBJDIR)_test

dir:
	mkdir -p $(BINDIR)
	cd $(SRCDIR) && find -type d -exec mkdir -p ../$(OBJDIR)/{} \;
	cd $(TESTDIR) && find -type d -exec mkdir -p ../$(OBJDIR)_test/{} \;

SRCS = $(shell cd $(SRCDIR) && find . -type f -name "*.cpp")
TESTSRCS = $(shell cd $(TESTDIR) && find . -type f -name "*.cpp")

OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)
OBJS_TEST = $(filter-out %main.o,$(OBJS)) $(TESTSRCS:%.cpp=$(OBJDIR)_test/%.o)


build: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEBUG) $(OBJS) -o $(BINDIR)/$(BINARY)

build-test: $(OBJS) $(OBJS_TEST)
	$(CXX) $(CXXFLAGS) $(TEST_LDFLAGS) $(DEBUG) $(OBJS_TEST) -o $(BINDIR)/test_$(BINARY)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG) -c $< -o $@

$(OBJDIR)_test/%.o: $(TESTDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG) -c $< -o $@
