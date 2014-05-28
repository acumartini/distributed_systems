CC=mpic++

rws: rws_mpi.cpp
	$(CC) -std=c++0x -o rws_mpi utils.cpp rws_mpi.cpp -Wall -fopenmp

all: rws_mpi

clean:
	rm -f rws_mpi
