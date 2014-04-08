
CXX 		= clang++
DEBUG		= -g -O0
OPTIMIZED	= -O3
CXXFLAGS	= -Weverything -std=c++11 -Wno-c++98-compat

SRCDIR		= src
BINDIR		= bin
OBJDIR		= build
BINARY		= db

default: dir build

#TODO opt build
debug: dir build

test: run-tests

happiness:
	@echo Sorry, not implemented yet.

clean:
	\rm -f $(BINDIR)/* $(OBJDIR)/*

dir:
	mkdir -p $(BINDIR) $(OBJDIR)

SRCS = $(shell cd $(SRCDIR) && find . -type f -name "*.cpp")
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

build: $(OBJS)
	$(CXX) $(CXXFLAGS) $(DEBUG) $(OBJS) -o $(BINDIR)/$(BINARY)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG) -c $< -o $@
