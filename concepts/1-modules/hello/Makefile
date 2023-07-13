# KDIR=/lib/modules/`uname -r`/build compiles with currently install kernel
# KDIR = /home/sadman/GitRepos/OS/linux compiles with a linux source repo  
KDIR ?= /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean: 
	rm *.mod.c *.o *.ko *.mod *.symvers *.order
	
clean-mod:	
	make -C $(KDIR) M=$(PWD) modules clean

.PHONY: all clean