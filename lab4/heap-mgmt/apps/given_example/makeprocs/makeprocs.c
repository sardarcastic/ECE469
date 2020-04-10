#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"

void main (int argc, char *argv[])
{
  int* a,b,c,d;

  if (argc != 1) {
    Printf("Usage: %s \n", argv[0]);
    Exit();
  }


  Printf("-------------------------------------------------------------------------------------\n");

  a = (int *) malloc(32);
  Printf("allcoated a\n");
  b = (int *) malloc(64);
  Printf("allcoated b\n");
  c = (int *) malloc(32);
  Printf("allcoated c\n");
  d = (int *) malloc(32);
  Printf("allcoated d\n");

  mfree((void *) b);
  Printf("Freed b\n");
  mfree((void *) d);
  Printf("Freed d\n");
  mfree((void *) a);
  Printf("Freed a\n");
  mfree((void *) c);
  Printf("Freed c\n");

  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
