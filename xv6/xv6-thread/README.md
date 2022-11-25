# XV6 Thread

Allows creation of threads in XV6 kernel. `clone()` system call creates a new thread within same address space as calling program. `join()` system call waits for a thread within same address space to complete. A spinlock is added to restrict concurrency in critical region. 

This project is described more [here](). 

Patch is from commit - e5da4b0b4f7b028369143abc4d319cdfd104169e