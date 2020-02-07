#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int numprocs = 5;
  int num_h20 = 0;
  int num_so4 = 0;
  int i;                          // Loop index variable
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  sem_t h20_sem, h2_sem, o2_sem, so4_sem, so2_sem, h2so4_sem;

  char h20_sem_str  [10],
       h2_sem_str   [10],
       o2_sem_str   [10],
       so4_sem_str  [10],
       so2_sem_str  [10],
       h2so4_sem_str[10]; // Used as command-line argument to pass page_mapped handle to new processes

  char procs_completed_str[10];
  
  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" 2\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  num_h20 = dstrtol(argv[1], NULL, 10);
  num_so4 = dstrtol(argv[2], NULL, 10);

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed = sem_create(-(numprocs-1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((h20_sem = sem_create(num_h20)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((so4_sem = sem_create(num_so4)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((h2_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((o2_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((so2_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((h2so4_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, procs_completed_str);
  ditoa(h20_sem, h20_sem_str);
  ditoa(so4_sem, so4_sem_str);
  ditoa(h2_sem, h2_sem_str);
  ditoa(o2_sem, o2_sem_str);
  ditoa(so2_sem, so2_sem_str);
  ditoa(h2so4_sem, h2so4_sem_str);

  react1_runs = num_h20/2;
  react2_runs = num_so4;
  //TODO: THIS IS WRONG
  react3_runs = (react2_runs < react1_runs) ? react2_runs : react1_runs; 

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(i=0; i<numprocs; i++) {
    process_create(PRODUCER_TO_RUN, h_mem_str, s_procs_completed_str, l_buff_str, empty_cv_str, full_cv_str, NULL);
    Printf("Producer %d created\n", i);
  }

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");
}
