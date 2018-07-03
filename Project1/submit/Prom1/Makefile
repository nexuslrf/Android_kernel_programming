obj-m := ptree.o
KID := /media/nexuslrf/b8feca26-e698-40ac-b002-3f6fe6563970/Android_project/android-kernel/kernel/goldfish
CROSS_COMPILE = arm-linux-androideabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
all:
	make -C $(KID) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) M=$(shell pwd) modules

clean:
	rm -rf *.ko *.o *.mod.c *.order *.symvers
