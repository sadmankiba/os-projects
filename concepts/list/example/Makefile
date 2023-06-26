# KDIR=/lib/modules/`uname -r`/build compiles with currently install kernel
# KDIR = /home/sadman/GitRepos/OS/linux compiles with a linux source repo  
KDIR ?= /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:	
	make -C $(KDIR) M=$(PWD) modules clean

.PHONY: all clean