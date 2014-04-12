CC=g++
CFLAGS=-I../__GXX_EXPERIMENTAL_CXX0X__ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -v -o "rws.o" "rws.cpp"
DEPS = 
OBJ = rws.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rwsmake: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o 