obj-m += chacha20.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod chacha20.ko
	sudo rmmod chacha20 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
