rm -f encrypt.o
rm -f modes.o
rm -f vpCamellia.o
rm -f camellia

gcc -I../header ../src/encrypt.c -mavx2 -c
gcc -I../header ../src/modes.c -mavx2 -c
gcc -I../header ../src/vpCamellia.c -mavx2 -c
gcc -o camellia encrypt.o modes.o vpCamellia.o
