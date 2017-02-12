
.PHONY: all
all:
	$(CXX) -O3 -std=c++11 src/cpu.cc src/libs/md5.cpp src/libs/SparseMatrix/SparseMatrix.cpp -Isrc -o O3.out
	#lscpu


.PHONY: test
test: all
	time ./O3.out ${ARGS}


.PHONY: valgrind
valgrind: all
	time valgrind --tool=cachegrind ./O3.out ${ARGS}


.PHONY: massif
massif: all
	time valgrind --tool=massif --massif-out-file=massif.out.12345 ./O3.out ${ARGS}
	ms_print massif.out.12345 | head -n 40


.PHONY: install
install:
	sudo apt-get install -qq g++-4.8
	export CXX="g++-4.8"


.PHONY: clear
clear:
	rm -f *.out 
	rm -f *.tmp
