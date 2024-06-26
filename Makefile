INCDIR = $(shell llvm-config --includedir)
LDFLAGS = $(shell llvm-config --libs --ldflags)

all: main
main: main.cpp
	clang++ -std=c++17 -I $(INCDIR) $(LDFLAGS) -lclang -o main main.cpp
	./main
	llvm-dis min.bc

mcc: cc.cpp
	clang++ -std=c++17 -I $(INCDIR) $(LDFLAGS) -lclang -lclang-cpp -o mcc cc.cpp

lexer: lexer-c.cpp
	clang++ -std=c++17 -I $(INCDIR) $(LDFLAGS) -lclang -o lexer lexer-c.cpp

minicc: minicc.cpp
	clang++ -std=c++17 -I $(INCDIR) $(LDFLAGS) -lclang-cpp -o minicc minicc.cpp

clanglibtest: clanglibtest.cpp
	clang++ -std=c++17 -I $(INCDIR) $(LDFLAGS) -lclang-cpp -lclang -o clanglibtest clanglibtest.cpp

clean:
	rm -f main
	rm -f *.ll
	rm -f *.bc
