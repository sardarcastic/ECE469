#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int i;
  mbox_t Co_mbox;
  mbox_t o2_mbox;
  mbox_t C2_mbox;
  char sending;
  char receiving;

  if (argc != 2) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_Co_mbox> <handle_to_o2_box> <handle_to_C2_mbox> <\n"); 
    Exit();
  } 

  Co_mbox = dstrtol(argv[1], NULL, 10);
  o2_mbox = dstrtol(argv[2], NULL, 10);
  C2_mbox = dstrtol(argv[3], NULL, 10);
  sending = ' ';

  //Receiving 4 messages
  for (i = 0; i<4; i++){
    if(mbox_recv(Co_mbox, 0, (void*)&receiving) == MBOX_FAIL){
      Printf("Failed to receive a message in react2. PID %d\n", getpid());
      Exit();
    }
  }
  //Sending 2 messages to o2_mbox and C2_mbox
  for (i = 0; i <2; i++){ 
    if (mbox_send(o2_mbox, 0, (void*)&sending) == MBOX_FAIL)
      Printf("Failed to send a message in react2. PID %d\n", getpid());
  }
  for (i = 0; i<2; i++){
    if (mbox_send(C2_mbox, 0, (void*)&sending) == MBOX_FAIL)
      Printf("Failed to send a message in react2. PID %d\n", getpid());
  }
  
  Exit();
}
