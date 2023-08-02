# Kernel Dev Concepts
## 7 - Device Driver

Create a logical device file 
```
mknod /dev/kbddv c 173 7
```

See major, minor numbers of devices
```
ls -l /dev/
```

See major numbers in order 
```
cat /proc/devices
```

Kernel driver output
```
[ 1350.836335] dev_driver init!
[ 1350.836397] sadman-dev registered
[ 1364.052547] sadman-dev opened
[ 1368.241148] sadman-dev reading at offset 0, size 100
[ 1368.241754] sadman-dev writing at offset 27, size 100
[ 1368.241791] Message in device data: You are reading sadman-dev!write msg from user
[ 1368.241827] sadman-dev ioctl called with cmd 1080323329
[ 1368.241857] Ioctl data: ioctl msg from user
```

## 8 - IO

Test ioport registration
```
cat /proc/ioports
```

Test interrupt registration
```
cat /proc/interrupts
```

Check number of interrupts on IRQs
```
cat /proc/stat | grep intr
```

# 10 - Memory Mapping 

Lower address in physical memory for kernel is mapped to userspace. After mapped, read, write system call completes very fast (~20x reduction). 

Surprisingly the 'userspace buffer' data does not change. 

User program 
```
mmap ret: 0x0x7f368a101000 (139872221270016)
```

Kernel program 
```
[ 3659.955317] mem_map init!
[ 3659.955373] memmap-dev registered
[ 3659.955394] memmap-dev added
[ 3659.955481] Allocated 5 pages
[ 3707.179740] memmap-dev opened
[ 3707.180351] mmap called, requested virt addr start: 0x7fd99035c000 (140572404072448), size: 4096
[ 3707.180389] alloced virt addr: 0000000038e1ffbe (18446627647122440192)
[ 3707.180410] alloced phy addr: 0x112a20000 (4607574016), pfn: 0x112a20 (1124896)
[ 3707.180420] Device data contains: Hello from kernel space!
[ 3707.180467] remap succeeded
[ 3707.180476] Now device data contains: Hello from kernel space!
[ 3707.180593] read called, requested len: 100, offset: 0
[ 3707.180606] Device data contains: Hello from kernel space!
[ 3707.180631] copy_to_user succeeded
[ 3707.180638] Now device data contains: Hello from kernel space!
[ 3707.180729] write called, requested len: 100, offset: 25
[ 3707.180742] Device data contains: Hello from kernel space!
[ 3707.180765] copy_from_user succeeded
[ 3707.180772] Now device data contains: Hello from kernel space!write msg from user
```

User program
```
Userspace buffer addr: 0x7ffd1c98d440 (140725083231296)
Message in userspace: Hello from user space!

mmaped. Now buffer addr: 0x7ffd1c98d440 (140725083231296)
mmap ret: 0x7fd99035c000 (140572404072448)
Now message in userspace buf: Hello from user space!

Message from kernel: Hello from kernel space!

Now message in userspace buf: Hello from user space!
```

