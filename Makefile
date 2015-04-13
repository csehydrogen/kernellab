obj-m += my_dev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod my_dev.ko
	sudo mknod /dev/my_dev c 1522 0
	gcc -o my_cli my_cli.c
	gcc -g -o my_msr_cli my_msr_cli.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rmmod my_dev
	sudo rm /dev/my_dev
	rm my_cli
	rm my_msr_cli
