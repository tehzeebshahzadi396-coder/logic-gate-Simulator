# Makefile for Digital Circuit & Logic Gate Simulator (C++17)
# Object Oriented Programming Project

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude

SRCDIR := src
TESTDIR := tests
BUILDDIR := build

LIB_SRCS := $(SRCDIR)/Wire.cpp \
            $(SRCDIR)/Gate.cpp \
            $(SRCDIR)/Gates.cpp \
            $(SRCDIR)/Circuit.cpp \
            $(SRCDIR)/NetlistParser.cpp \
            $(SRCDIR)/TruthTableGenerator.cpp

LIB_OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(LIB_SRCS))

MAIN_SRC := $(SRCDIR)/main.cpp
MAIN_OBJ := $(BUILDDIR)/main.o

TEST_SRC := $(TESTDIR)/test_main.cpp
TEST_OBJ := $(BUILDDIR)/test_main.o

EXEC := logic_sim
TEST_EXEC := run_tests

ifeq ($(OS),Windows_NT)
    EXEC := logic_sim.exe
    TEST_EXEC := run_tests.exe
    MKDIR := if not exist $(BUILDDIR) mkdir $(BUILDDIR)
    RM := if exist $(BUILDDIR) rmdir /s /q $(BUILDDIR) & if exist $(EXEC) del $(EXEC) & if exist $(TEST_EXEC) del $(TEST_EXEC)
else
    MKDIR := mkdir -p $(BUILDDIR)
    RM := rm -rf $(BUILDDIR) $(EXEC) $(TEST_EXEC)
endif

.PHONY: all test clean

all: $(EXEC) $(TEST_EXEC)

$(BUILDDIR):
	@$(MKDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(TESTDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXEC): $(MAIN_OBJ) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_EXEC): $(TEST_OBJ) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(TEST_EXEC)
	./$(TEST_EXEC)

clean:
	@$(RM)
