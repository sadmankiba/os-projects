# XV6 Thread

Allows creation of threads in XV6 kernel. `clone()` system call creates a new thread within same address space as calling program. `join()` system call waits for a thread within same address space to complete. A spinlock is added to restrict concurrency in critical region. 