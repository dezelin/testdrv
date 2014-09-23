testdrv
==========

FIFO characted device driver implementation for the interview.

## Build

    cd src/
    make

## Test

    cd src/
    make
    make install
    ./test_stream.h

This will generate a file filled with random data in /tmp directory and stream it
to another file through the driver. SHA128 sums of those two files are compared after streaming.

