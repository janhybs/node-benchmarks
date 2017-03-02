
.PHONY: compile
compile:
	$(CXX) -O3 -std=c++11 cpu.cc libs/md5.cpp libs/SparseMatrix/SparseMatrix.cpp -I./ -o O3.out
	#lscpu


.PHONY: test
test: compile
	time ./O3.out ${ARGS} $(JSON) $(ERROR) $(VERSION)


	
.PHONY: test-fast
test-fast:
	time ./O3.out ${ARGS} $(JSON) $(ERROR) $(VERSION)



.PHONY: valgrind
valgrind: all
	time valgrind --tool=cachegrind ./O3.out ${ARGS} $(JSON) $(ERROR) $(VERSION)


.PHONY: massif
massif: all
	time valgrind --tool=massif --massif-out-file=massif.out.12345 ./O3.out ${ARGS} $(JSON) $(ERROR) $(VERSION)
	ms_print massif.out.12345 | head -n 40


.PHONY: install
install:
	sudo apt-get install -qq g++-4.8
	export CXX="g++-4.8"


.PHONY: clear
clear:
	rm -f *.out 
	rm -f *.tmp
