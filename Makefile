default: rws

tauTest: rws.c
    gcc-4.8 -o rws rws.c

clean:
    rm *.o