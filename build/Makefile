all: camellia

camellia: vpCamellia.o encrypt.o modes.o
	gcc -o camellia encrypt.o modes.o vpCamellia.o

vpCamellia.o: ../src/vpCamellia.c
	gcc -I../header ../src/vpCamellia.c -mavx2 -c

encrypt.o: ../src/encrypt.c
	gcc -I../header ../src/encrypt.c -mavx2 -c

modes.o: ../src/modes.c
	gcc -I../header ../src/modes.c -mavx2 -c

clean:
	rm -rf *.o camellia
