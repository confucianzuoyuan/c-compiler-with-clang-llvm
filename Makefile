INCDIR = $(shell llvm-config --includedir)
LDFLAGS = $(shell llvm-config --libs --ldflags)

all: main
main: main.cpp
	clang++ -std=c++17 -I $(INCDIR) $(LDFLAGS) -lclang -o main main.cpp
	./main
	llvm-dis min.bc

parser: parser.cpp
	clang++ -std=c++17 -I $(INCDIR) $(LDFLAGS) -lclang -o parser parser.cpp

clean:
	rm -f main
	rm -f *.ll
	rm -f *.bc