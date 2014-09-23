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

    This will generate a file filled with random data in /tmp directory, stream it
through the driver file /dev/testdrv and save it to another file in /tmp directory.
    SHA sums of those two files are compared after that.

