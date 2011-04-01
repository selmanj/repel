# handwritten since I can't get the autogen to work

BOOSTLIB=/scratch/boost_1_46_1

spanintervaltest: spanintervaltest.o ../src/spaninterval.o ../src/interval.o
	g++ -o spanintervaltest spanintervaltest.o ../src/spaninterval.o ../src/interval.o -I../src -I$(BOOSTLIB)

spanintervaltest.o: spanintervaltest.cpp ../src/spaninterval.cpp ../src/spaninterval.h ../src/interval.cpp ../src/interval.h
	g++ -c spanintervaltest.cpp	-I../src/ -I$(BOOSTLIB)

.PHONY : clean
clean : 
	rm spanintervaltest spanintervaltest.o 
