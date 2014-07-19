all:
	g++ -std=c++11 -I. -I/usr/lib/llvm-3.5/include -L/usr/lib/llvm-3.5/lib -lclang -o test test.cpp

run:
	./test
