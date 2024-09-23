
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -Wall -g

all: correctness persistence

correctness: skiplist.o sstableHeader.o bloomFilter.o ssTable.o ssTableCache.o kvstore.o correctness.o

persistence: skiplist.o sstableHeader.o bloomFilter.o ssTable.o ssTableCache.o kvstore.o persistence.o

clean:
	-rm -f correctness persistence *.o
