
DRIVERNAME = testdrv

obj-m += $(DRIVERNAME).o
$(DRIVERNAME)-objs += device.o quantum_queue.o

all:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
		sudo mknod -m 666 /dev/testdrv c 250 0
		sudo insmod testdrv.ko

uninstall:
		sudo rmmod testdrv
		sudo rm -f /dev/testdrv

test:
		./test_stream.sh

