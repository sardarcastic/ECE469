#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int i;
  mbox_t S_mbox;
  mbox_t o2_mbox;
  mbox_t So4_mbox;
  char sending;
  char receiving;
  if (argc != 2) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_S_mbox> <handle_to_o2_mbox> <handle_to_So4_mbox> <\n"); 
    Exit();
  } 

  S_mbox = dstrtol(argv[1], NULL, 10);
  o2_mbox = dstrtol(argv[2], NULL, 10);
  So4_mbox = dstrtol(argv[3], NULL, 10);
  sending = ' ';

  //Receiving 3 messages
  if(mbox_recv(S_mbox, 0, (void*)&receiving) == MBOX_FAIL){
    Printf("Failed to receive a message in react3. PID %d\n", getpid());
    Exit();
  }
  for (i = 0; i<2; i++){
    if(mbox_recv(o2_mbox, 0, (void*)&receiving) == MBOX_FAIL){
      Printf("Failed to receive a message in react3. PID %d\n", getpid());
      Exit();
    }
  }
  //Sending 1 message to So4 mbox 
  if (mbox_send(So4_mbox, 0, (void*)&sending) == MBOX_FAIL) {
    Printf("Failed to send a message in react3. PID %d\n", getpid());
  }
  
  Exit();
}
