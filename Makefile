LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++20 -Wall

all: bin bin/correctness bin/persistence bin/main

bin: 
	mkdir -p bin

correctness: bin src/skiplist/skiplist.o src/bloomfilter/bloomfilter.o src/sstable/sstable.o src/kvstore/kvstore.o src/tests/correctness.o
	$(LINK.o) -o bin/correctness src/skiplist/skiplist.o src/bloomfilter/bloomfilter.o src/sstable/sstable.o src/kvstore/kvstore.o src/tests/correctness.o

persistence: bin src/skiplist/skiplist.o src/bloomfilter/bloomfilter.o src/sstable/sstable.o src/kvstore/kvstore.o src/tests/persistence.o
	$(LINK.o) -o bin/persistence src/skiplist/skiplist.o src/bloomfilter/bloomfilter.o src/sstable/sstable.o src/kvstore/kvstore.o src/tests/persistence.o

main: bin src/skiplist/skiplist.o src/bloomfilter/bloomfilter.o src/sstable/sstable.o src/kvstore/kvstore.o src/tests/main.o
	$(LINK.o) -o bin/main src/skiplist/skiplist.o src/bloomfilter/bloomfilter.o src/sstable/sstable.o src/kvstore/kvstore.o src/tests/main.o

clean: 
	-rm -f bin/* src/*/*.o
