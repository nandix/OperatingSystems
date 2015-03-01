
CC = gcc

CXXFLAGS = -Wall -pthread -g

EXE = dsh

all: $(EXE)

dsh: dsh.c prog1.c prog2.c prog3.c helperfunctions.c
	$(CC) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o $(EXE)

