# Makefile for CNTCL - Compile-Time Number Theory & Combinatorics Library

CXX = clang++

# Detect architecture
ARCH := $(shell uname -m)

# Base flags
CXXFLAGS = -std=c++20 -Wall -Wextra -O3 -fcoroutines-ts
INCLUDES = -I./include
LIBS = -pthread

# Architecture-specific flags
ifeq ($(ARCH),arm64)
    # Apple Silicon (M1/M2/M3)
    CXXFLAGS += -mcpu=apple-m1
else ifeq ($(ARCH),x86_64)
    # Intel Mac
    CXXFLAGS += -march=native
endif

# Directories
SRC_DIR = src
INC_DIR = include
TEST_DIR = tests
BUILD_DIR = build
LIB_DIR = lib

# Files
HEADERS = $(INC_DIR)/CNTCL.hpp
TEST_SRC = $(TEST_DIR)/test_CNTCL.cpp
TEST_EXE = $(BUILD_DIR)/test_CNTCL

# Targets
.PHONY: all clean test benchmark

all: directories $(TEST_EXE)

directories:
	mkdir -p $(BUILD_DIR) $(LIB_DIR)

$(TEST_EXE): $(TEST_SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(TEST_SRC) $(LIBS)

test: $(TEST_EXE)
	$(TEST_EXE)

benchmark: $(TEST_EXE)
	$(TEST_EXE) --benchmark

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)