#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int *x;
  int *q;
  int z;
  
  // Accessing memory beyond max virtual address

  Printf("About to access memory beyond the maximum virtual addres (PID: %d)\n", getpid());
  x = malloc(1000);
  q = malloc(1000);
  x = q + 0xfffff;
  z = &x;

  Printf("process should have aborted (PID: %d) (should not have printed)\n", getpid());

  
}
