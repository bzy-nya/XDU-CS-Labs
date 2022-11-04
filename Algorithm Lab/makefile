TARGETS := ./targets

build: all unitest

run: build
	./unitest

clean:
	rm *.o

unitest: *.cc unitest.cpp bzy.h
	g++ -std=c++20 -o unitest *.o unitest.cpp

all:$(subst .cc,.o,$(wildcard *.cc))
%.o:%.cc bzy.h
	g++ -std=c++20 -c -o $@ $<
