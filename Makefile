CXX ?= g++
CXXFLAGS += -std=c++17 -Wall -Wextra -O2 -Iincludes
LDFLAGS += -lsqlite3

# CUnit: pkg-config, иначе Homebrew (brew install cunit), иначе системные пути
HAS_PKG_CUNIT := $(shell pkg-config --exists cunit 2>/dev/null && echo 1)
ifeq ($(HAS_PKG_CUNIT),1)
  CUNIT_CFLAGS := $(shell pkg-config --cflags cunit)
  CUNIT_LIBS := $(shell pkg-config --libs cunit)
else
  BREW_CUNIT := $(shell brew --prefix cunit 2>/dev/null)
  ifneq ($(BREW_CUNIT),)
    CUNIT_CFLAGS := -I$(BREW_CUNIT)/include
    CUNIT_LIBS := -L$(BREW_CUNIT)/lib -lcunit
  else
    CUNIT_CFLAGS :=
    CUNIT_LIBS := -lcunit
  endif
endif

TEST_CXXFLAGS = $(CXXFLAGS) $(CUNIT_CFLAGS)
TEST_LDFLAGS = $(LDFLAGS) $(CUNIT_LIBS)

SRCDIR = src
BUILDDIR = build
BINDIR = bin
SQLDIR = sql
DB_PATH = $(BUILDDIR)/helicopter.db
SQLITE3 ?= sqlite3

APP_SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/db.cpp $(SRCDIR)/auth.cpp $(SRCDIR)/queries.cpp $(SRCDIR)/mutations.cpp $(SRCDIR)/payroll.cpp $(SRCDIR)/storage.cpp
APP_OBJS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(APP_SRCS))

LIB_OBJS = $(BUILDDIR)/db.o $(BUILDDIR)/auth.o $(BUILDDIR)/queries.o $(BUILDDIR)/mutations.o $(BUILDDIR)/payroll.o $(BUILDDIR)/storage.o

.PHONY: all clean test dirs run init-db

all: dirs $(BINDIR)/helicopter_app $(DB_PATH)

dirs:
	@mkdir -p $(BUILDDIR) $(BINDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR)/helicopter_app: $(APP_OBJS)
	$(CXX) -o $@ $(APP_OBJS) $(LDFLAGS)

$(DB_PATH): $(SQLDIR)/HelicopterDelivery_create.sql $(SQLDIR)/extensions.sql $(SQLDIR)/seed.sql | dirs
	rm -f $@
	$(SQLITE3) $@ < $(SQLDIR)/HelicopterDelivery_create.sql
	$(SQLITE3) $@ < $(SQLDIR)/extensions.sql
	$(SQLITE3) $@ < $(SQLDIR)/seed.sql

init-db: $(DB_PATH)

run: all
	HELI_SQL_DIR=$(SQLDIR) HELI_SEED=0 ./$(BINDIR)/helicopter_app $(DB_PATH)

# --- CUnit tests ---
$(BUILDDIR)/test_db.o: tests/test_db.cpp | dirs
	$(CXX) $(TEST_CXXFLAGS) -c tests/test_db.cpp -o $@

$(BINDIR)/test_db: $(LIB_OBJS) $(BUILDDIR)/test_db.o
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)

$(BUILDDIR)/test_auth.o: tests/test_auth.cpp | dirs
	$(CXX) $(TEST_CXXFLAGS) -c tests/test_auth.cpp -o $@

$(BINDIR)/test_auth: $(LIB_OBJS) $(BUILDDIR)/test_auth.o
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)

$(BUILDDIR)/test_queries.o: tests/test_queries.cpp | dirs
	$(CXX) $(TEST_CXXFLAGS) -c tests/test_queries.cpp -o $@

$(BINDIR)/test_queries: $(LIB_OBJS) $(BUILDDIR)/test_queries.o
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)

$(BUILDDIR)/test_mutations.o: tests/test_mutations.cpp | dirs
	$(CXX) $(TEST_CXXFLAGS) -c tests/test_mutations.cpp -o $@

$(BINDIR)/test_mutations: $(LIB_OBJS) $(BUILDDIR)/test_mutations.o
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)

$(BUILDDIR)/test_payroll.o: tests/test_payroll.cpp | dirs
	$(CXX) $(TEST_CXXFLAGS) -c tests/test_payroll.cpp -o $@

$(BINDIR)/test_payroll: $(LIB_OBJS) $(BUILDDIR)/test_payroll.o
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)

$(BUILDDIR)/test_storage.o: tests/test_storage.cpp | dirs
	$(CXX) $(TEST_CXXFLAGS) -c tests/test_storage.cpp -o $@

$(BINDIR)/test_storage: $(LIB_OBJS) $(BUILDDIR)/test_storage.o
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)

$(BUILDDIR)/test_main.o: tests/test_main.cpp | dirs
	$(CXX) $(TEST_CXXFLAGS) -c tests/test_main.cpp -o $@

$(BINDIR)/test_main: $(BUILDDIR)/test_main.o
	$(CXX) -o $@ $^ $(LDFLAGS) $(CUNIT_LIBS)

TEST_BINS = $(BINDIR)/test_db $(BINDIR)/test_auth $(BINDIR)/test_queries $(BINDIR)/test_mutations $(BINDIR)/test_payroll $(BINDIR)/test_storage $(BINDIR)/test_main

test: all $(TEST_BINS)
	HELI_SQL_DIR=sql HELI_SEED=1 $(BINDIR)/test_db
	HELI_SQL_DIR=sql HELI_SEED=1 $(BINDIR)/test_auth
	HELI_SQL_DIR=sql HELI_SEED=1 $(BINDIR)/test_queries
	HELI_SQL_DIR=sql HELI_SEED=1 $(BINDIR)/test_mutations
	HELI_SQL_DIR=sql HELI_SEED=1 $(BINDIR)/test_payroll
	HELI_SQL_DIR=sql HELI_SEED=1 $(BINDIR)/test_storage
	$(BINDIR)/test_main

clean:
	rm -rf $(BUILDDIR) $(BINDIR)
