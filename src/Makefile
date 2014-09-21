
DRIVERNAME = testdrv

obj-m += $(DRIVERNAME).o
$(DRIVERNAME)-objs += device.o quantum_queue.o

all:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
