OPTIONS=-Wall -fPIC -O2 -g

all: shared

ptp.o:
	gcc -c ${OPTIONS} ptp.c

shared: ptp.o
	gcc -shared -Wl,-soname,libptp.so -o libptp.so ptp.o -lpthread -lrt

test: shared
	gcc main.c -g -o ptp -L. -lptp

install:
	cp libptp.so /usr/lib/

clean:
	rm *.o
	rm *~
	rm ptp
	rm *.so*