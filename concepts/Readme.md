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