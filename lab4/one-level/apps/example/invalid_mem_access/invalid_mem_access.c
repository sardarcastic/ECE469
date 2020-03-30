#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  int *x;
  int *q;
  int z;
  
  // Performing an invalid memory access on memory outside of currently allocated page
  Printf("About to perform invalid memory access outside of allocated page (PID: %d)\n", getpid());

  x = malloc(800);
  q = x + 0xfff + 1;

  z = 1 + &q;

  Printf("THis should not print (PID: %d)\n", getpid());


}
