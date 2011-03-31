objects = interval.o spaninterval.o pel.o

pel : $(objects)
	g++ -o pel $(objects)

interval.o : interval.h interval.cpp
spaninterval.o : spaninterval.h spaninterval.cpp interval.h interval.cpp
pel.o : pel.cpp

.PHONY : clean
clean :
	rm pel $(objects)
