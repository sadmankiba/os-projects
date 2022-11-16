# XV6 Notes

## Run 

Build: 
`make`.
For OS X, see https://pdos.csail.mit.edu/6.828/2011/tools.html. Download `binutils` and install.

Then, run simulator: `make qemu` or `make qemu-nox`. Gives you a shell in a custom filesystem of XV6.

Shell doesn't take `exit` command. So, I kill it as `kill -9 <pid of make qemu-nox>`. Then, `make clean`. 

## Code Structure
The source code can be roughly devided into user space and kernel space code.  

### User Space 
Starts at `main.c`. Initializes interrupt controller, page table, process table among others. The emulator provides a shell (`sh.c`). When a command is entered in shell, forks and runs that command. Commands can be programs like `ls.c`, `ln.c`, `wc.c` etc. and can contain redirections, pipes etc. User level handy functions such as `strcpy` and `atoi` are defined in `ulib.c`. 

System calls from user space are declared in `user.h` and are defined in assembly `usys.S`. Each syscall does same in that it moves it syscall number to `eax` register and calls an interrupt. Arguments are not passed to kernel-level functions. Rather, they are kept on stack.

### Kernel Space 
System calls are performed in `syscall.c`. It reads `eax` register and calls the appropriate syscall handler. `sysfile.c` also defines functions to retrieve arguments from stack for syscalls. Syscall handlers in kernel space are defined in `sysfile.c` with functions like `sys_open`, `sys_mkdir` etc.. 

## Exercises

### P1. Write a user-level program that prints "hello <name>".

### P2. Write a user-level program that uses `fork()` and `wait()`. 



## Exercise Solutions

### P1 - hello

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

### P2 - fork 

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

Run same as P1 sol.