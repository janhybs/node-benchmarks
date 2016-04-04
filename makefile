.PHONY: all
all:
	$(CXX) -O0 -std=c++11 src/memory.cc -o O0.out
	lscpu

.PHONY: O0.out
O0.out:
	 ./O0.out
 
.PHONY: test
test: O0.out
 
.PHONY: install
install:
	sudo apt-get install -qq g++-4.8
	export CXX="g++-4.8"

.PHONY: clear
clear:
	rm -f *.out 