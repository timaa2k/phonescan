SOURCE=main.cpp
PROGRAM=phonescan
LIBRARIES=-lpthread -ltins
CC=g++
FLAGS=-std=c++11 -O3

#--------------------------------------------

all: $(PROGRAM)

$(PROGRAM): $(SOURCE)

	$(CC) $(FLAGS) $(SOURCE) -o$(PROGRAM) $(LIBRARIES)

clean:

	rm -f *.o
	rm -f $(PROGRAM)

#===========================================
