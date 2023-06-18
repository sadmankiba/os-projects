# Use KDIR=/lib/modules/`uname -r`/build to compile with currently install kernel
KDIR=/home/sadman/GitRepos/OS/linux

all:
	make -C $(KDIR) M=$(PWD) modules

clean:	
	make -C $(KDIR) M=$(PWD) modules clean

.PHONY: all clean