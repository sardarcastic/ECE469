#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int i;
  mbox_t S2_mbox;
  char *sending;
  if (argc != 2) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_S2_mbox> \n"); 
    Exit();
  } 

  S2_mbox = dstrtol(argv[1], NULL, 10);
  sending = ' ';
  
  if (mbox_send(S2_mbox, 0, (void*)sending) == MBOX_FAIL)
    Printf("Failed to send a message in S2Gen. PID %d\n", getpid());

  Exit();
}
