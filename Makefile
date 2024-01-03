all: sodoku leetcode.o

sodoku : sodoku.cc Makefile
	clang++ -g -std=c++2a -Wall -o $@ $<

leetcode.o : leetcode.cc Makefile
	clang++ -c -g -std=c++2a -Wall $<

clean :
	rm -rf sodoku leetcode.o leetcode.dSYM sodoku.dSYM


.PHONY: run
run: sodoku
	time ./sodoku
