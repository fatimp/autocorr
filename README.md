# autocorr

Autocorr is a research library for computing autocorrelations of binary
sequences (sequences with elements being equal to 0 or 1) using FFT over a
finite field.

## How to build

Requirements:

* Lua 5.4 (actually, any 5.x version will do with a small modification of the
  code gen script.)
* CMake
* CUnit (for tests)

Build instructions:

1. cd generation && sh generate.sh
2. cd .. && mkdir build
3. cd build && cmake -DCMAKE_BUILD_TYPE=RELEASE ..
4. make

## How to use

The library has a simple interface contained in a single header
(`autocorr.h`). Here is a typical workflow:

1. First of all, you need to allocate the buffers for input and output data (the
algorithm can be inplace, actually. A decision not to modify the input is
deliberate). The allocation can be done with `ac_alloc` which accepts a number
of elements in the input sequence.
2. Then, you obtain pointers to the input and output buffers with `ac_get_src`
and `ac_get_dst`.
3. Copy your data to the input buffer.
4. Call `ac_autocorr` to compute the result.
5. Read the result from the output buffer. Both the input and output buffers
   have the same length which can be accessed via
   `ac_autocorr_length`. Currently its `2n - 1` ceiled to the closest power
   of 2. The autocorrelation has a symmetry of a sort. E.g. for the input of
   length 4 the result is: `ac[1] ac[2] ac[3] ac[4] ac[3] ac[2] ac[1] ac[0]`.
6. Free buffers with `ac_free`.

## Simple example

An example program is built with the library. It can be used like this:

~~~~
$ autocorr-example 1 1 1 1 0 1
3 3 2 1 1 0 0 0 0 0 1 1 2 3 3 5
~~~~

## Performance

Calculating autocorrelation of a sequence of 65536 elements 5000 times on my
Ryzen 7 5800X CPU gives:

* 54 seconds (FFTW)
* **43 seconds (autocorr)**
* 18 seconds (real input FFT, FFTW)

## Reading

* Geddes, Keith O., Stephen R. Czapor, and George Labahn. Algorithms for
  computer algebra. Springer Science & Business Media, 1992.
* Discrete-Time Signal Processing, 3-rd edition by Alan Oppenheim and Ronald
  Schafer
