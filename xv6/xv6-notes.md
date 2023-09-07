# XV6 Notes

## Brief

XV6 is a fully-functional OS kernel. It supports
- system calls such as `open()` and `fork()`.
- virtualizing memory (with paging support) and cpu (round-robin scheduling)
- filesystem. 

Does not support 
- Concurrency with threads.

## Run 

**Build** 
`make`.
For OS X, see https://pdos.csail.mit.edu/6.828/2011/tools.html. Download `binutils` and install.

**Running simulator**
After building, run: `make qemu` or `make qemu-nox`. Gives you a shell in a custom filesystem of XV6.

**Stopping**

Shell doesn't take `exit` command. So, kill it as `killall -r qemu` or `kill -9 <pid of make qemu-nox>`. Then, `make clean`. 


**Debug**
- In xv6, use `cprintf` to print your debug lines.
- To debug with gdb, run `make qemu-nox-gdb` or `make qemu-gdb` in one terminal go to xv6 directory, and `gdb` in another terminal.
- add auto-load-save-path.

## Code Structure
The source code can be roughly devided into user space and kernel space code.  

### User Space 
Starts at `main.c`. Initializes interrupt controller, page table, process table among others. The emulator provides a shell (`sh.c`). When a command is entered in shell, forks and runs that command. Commands can be programs like `ls.c`, `ln.c`, `wc.c` etc. and can contain redirections, pipes etc. User level handy functions such as `strcpy` and `atoi` are defined in `ulib.c`. 

System calls from user space are declared in `user.h` and are defined in assembly `usys.S`. Each syscall works similarly - moves syscall number to `eax` register and calls an interrupt. Arguments are not passed to kernel-level functions. Rather, they are kept on stack.

### Kernel Space 
System calls go through trap and finally to `syscall.c`. It reads `eax` register and calls the appropriate syscall handler. `sysfile.c` also defines functions to retrieve arguments from stack for syscalls. Syscall handlers in kernel space are defined in `sysfile.c` with functions like `sys_open`, `sys_mkdir` etc.. 

`proc.c` contains process-related methods such as `userinit` to create init process, `fork` etc. `proc.h` contains process state struct `proc`. 

## Exercises

### P1. Write a user-level program. 
  1.1. prints "hello <name>".
  1.2. uses `fork()` and `wait()`. 

### P2. Add a system call. 

## Exercise Solutions

### P1 
1.1 hello

hello.c

```c
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "hello %s\n", argv[1]);
  exit();
}
```

Add `_hello` to UPROGS in Makefile. Build and run emulator. You can now run `$ hello <name>` in shell.

1.2 fork 

checkfork.c
```c
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid = fork();
  if (pid == 0) {
    printf(1, "In child...\n");
    exit();
  }
  wait();
  printf(1, "Child finished. In parent...\n");
  exit();
}
```

Run same as P1.1 sol.

### P2

user.h, usys.S, syscall.h, syscall.c, sysfile.c, proc.c