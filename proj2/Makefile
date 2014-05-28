CC=mpic++

rws: rws_mpi.cpp
	$(CC) -std=c++0x -o rws_mpi -O2 utils.cpp rws_mpi.cpp -Wall -Wextra -fopenmp

all: rws_mpi

clean:
	rm -f rws_mpi
