LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++20 -Wall

BINDIR = bin
SRCDIR = src
TESTDIR = tests

all: $(BINDIR)/correctness $(BINDIR)/persistence

$(BINDIR): 
	mkdir -p $(BINDIR)

$(BINDIR)/correctness: src/kvstore/kvstore.o src/tests/correctness.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ src/kvstore/kvstore.o src/tests/correctness.o

$(BINDIR)/persistence: src/kvstore/kvstore.o src/tests/persistence.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ src/kvstore/kvstore.o src/tests/persistence.o

clean: 
	-rm -f $(BINDIR)/correctness $(BINDIR)/persistence src/kvstore/*.o src/tests/*.o