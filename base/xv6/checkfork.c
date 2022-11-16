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
