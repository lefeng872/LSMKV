
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++20 -Wall

all: correctness persistence

correctness: src/kvstore/kvstore.o src/tests/correctness.o

persistence: src/kvstore/kvstore.o src/tests/persistence.o

clean:
	-rm -f correctness persistence *.o
