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

Lower address in physical memory for kernel is mapped to userspace.

User program 
```
mmap ret: 0x0x7f368a101000 (139872221270016)
```

Kernel program 
```
[ 2886.208412] mmap called, requested virt addr start: 0x7f368a101000 (139872221270016), size: 4096
[ 2886.208412] alloced virt addr: 000000001dfc5a2a (503077418)
[ 2886.208412] alloced phy addr: 0x34cf8000 (886013952), pfn: 0x34cf8 (216312)
```