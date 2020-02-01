# vpcamellia
An implementation of camellia through vector permute instructions. This work is done based on AES implementation in openSSL, which uses vector permute instructions for s-box and mixcolumn operations. Similarly, this implementation uses AVX2 instructions for s-box, p function and Fl,FL<sup>-1</sup> operations.

To build this repository, move inside the build directory and start the build.sh file.
```
cd build
make
```

For this code to work, your machine must support AVX2 instruction set.
To check that, issue the following command from terminal.
```
lscpu
```
In the output of lscpu, check if AVX2 is available in flags section or else you can grep as follows
```
lscpu | grep AVX2
```

If your machine didn't support AVX2, don't worry as there are a couple of other ways to make it work.
1. You can use Intel's emulator(also called SDE) to make it work.
2. You can also use some tool, like QEMU, which can simulate these instruction sets.
