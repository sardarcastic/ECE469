#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  int *x;
  int q;
  int *z;
  
  // Performing an invalid memory access on memory outside of currently allocated page
  Printf("About to perform invalid memory access outside of allocated page (PID: %d)\n", getpid());

  x = malloc(4);
  if (x == -1)
    Printf("malloc call didn't work\n");
  else
    *x = 8;

  q = 7;
  Printf("The address of allocated memory is: %d (the value is: %d)\n", &q, q);
  z = &x + 0xfff0;
  Printf("(SHOULD NOT PRINT) new add: %d (new val: %d)\n", z, *z);


  Printf("This should not print (PID: %d)\n", getpid());


}
