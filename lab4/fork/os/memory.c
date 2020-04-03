//o
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "memory_constants.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
static uint32 freemap[MEM_FREEMAP_SIZE];
static uint8 pageRefCounter[MEM_REF_COUNTER_SIZE]; //should only be 512 elements (for each page available)

static uint32 pagestart;
static int nfreepages;
static int freemapmax;

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryModuleInit
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//      0 bit in freemap array means page is in use - "VALID" ?
//      1 bit means not in use
//
//----------------------------------------------------------------------
void MemoryModuleInit() {
  int i;
  pagestart = lastosaddress / MEM_PAGE_SIZE;
  freemapmax = MemoryGetSize() / MEM_PAGE_SIZE;
  nfreepages = freemapmax - pagestart;

  if (lastosaddress % MEM_PAGE_SIZE != 0) pagestart++;

  for (i = 0; i < MEM_FREEMAP_SIZE; i++) {
    freemap[i] = 0;
  }

  for (i = pagestart; i < freemapmax; i++) {
    MemorySetFreemap(i);
  }
  // printf("MemoryModuleInit: nfreepages %d\n", nfreepages);
  for (i = 0; i < MEM_REF_COUNTER_SIZE; i++)
    pageRefCounter[i] = 0;
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
  uint32 pagenum = addr >> MEM_L1FIELD_FIRST_BITNUM;
  uint32 offset = addr & MEM_ADDRESS_OFFSET_MASK;

  if (addr > MEM_MAX_VIRTUAL_ADDRESS) {
    printf("PID: %d addr is larger than possible virtual address\n", GetPidFromAddress(pcb));
    printf("  killing PID: %d\n", GetCurrentPid());
    ProcessKill();
  }

  if ((pcb->pagetable[pagenum] & MEM_PTE_VALID) != MEM_PTE_VALID) {
    pcb->currentSavedFrame[PROCESS_STACK_FAULT] = addr;
    MemoryPageFaultHandler(pcb);
  }

  //printf("MemoryTranslate: %x\n", (pcb->pagetable[pagenum] & MEM_PTE_MASK) | offset);
  return (pcb->pagetable[pagenum] & MEM_PTE_MASK) | offset;
}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);

    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference.
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
  uint32 npg;
  uint32 fault_addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
  uint32 usr_stack_addr = pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER];
  uint32 fault_pagenum = fault_addr >> MEM_L1FIELD_FIRST_BITNUM;
  uint32 stack_pagenum = usr_stack_addr >> MEM_L1FIELD_FIRST_BITNUM;

  printf("MEMFaultHandler %x %x \n", fault_addr, usr_stack_addr);
  if (fault_pagenum >= MEM_L1TABLE_SIZE) {
    printf("PID: %d: 0x%x address exceeds maximum virtual address\n", fault_addr, GetPidFromAddress(pcb));
    printf("  killing PID: %d\n", GetCurrentPid());
    ProcessKill();
    return MEM_FAIL;
  }

  if (fault_pagenum < stack_pagenum) {
    printf("PID: %d SEGFAULT\n", GetPidFromAddress(pcb));
    printf("  killing PID: %d\n", GetCurrentPid());
    ProcessKill();
    return MEM_FAIL;
  }
  else {
    printf("MemoryPageFaultHandler: Growing call stack of PID: %d\n", GetPidFromAddress(pcb));
    npg = MemoryAllocPageEasy(pcb);
    pcb->pagetable[fault_pagenum] = MemorySetupPte(npg);
    pcb->npages++;
    return MEM_SUCCESS;
  }
}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

int MemoryAllocPageEasy(PCB *pcb) {
  uint32 pagenum;
  if ((pagenum = MemoryAllocPage()) == 0) {
    // printf("MemoryAllocPageEasy: killing pagenum: %d\n", pagenum);
    // printf("MemoryAllocPageEasy: killing nfreepages: %d\n", nfreepages);
    printf("MemoryAllocPageEasy: PID: %d no more pages to allocate\n", GetPidFromAddress(pcb));
    printf("  killing PID: %d\n", GetCurrentPid());
    ProcessKill();
  }
  // incrementing the ref counter for this particular page
  pageRefCounter[pagenum]++;
  return pagenum;
}

int MemoryAllocPage(void) {
  int i;
  int j;
  uint32 tmp;
  if (nfreepages == 0) {
    // printf("MemoryAllocPage: returning zero nfreepags: %d\n", nfreepages);
    return 0;
  }

  for (i = 0; i < MEM_FREEMAP_SIZE; i++) {
    if (freemap[i] != 0) {
      tmp = freemap[i];
      for (j = 0; j < 32; j++) {
        if ((tmp & 0x1) == 1) {
          nfreepages--;
	  // printf("MemoryAllocPage: nfreepags: %d\n", nfreepages);
	  freemap[i] &= invert(0x1 << j);
          return (i*32 + j);
        }
        tmp = tmp >> 1;
      }
    }
  }
  return 0;
}

void MemorySetFreemap(uint32 page) {
  freemap[page / 32] |= 1 << (page % 32);
}

void MemoryFreePage(uint32 page) {
  pageRefCounter[page]--;
  // If no more processes are using this page, then we can set it as free
  if (pageRefCounter[page] == 0){
    MemorySetFreemap(page);
    nfreepages++;
  }
    
}

void MemoryFreePte(uint32 pte) {
  MemoryFreePage((pte & MEM_PTE_MASK) >> MEM_L1FIELD_FIRST_BITNUM);
}

uint32 MemorySetupPte (uint32 page) {
  //printf("MemorySetupPte: %x\n", (page << MEM_L1FIELD_FIRST_BITNUM) | MEM_PTE_VALID);
  return (page << MEM_L1FIELD_FIRST_BITNUM) | MEM_PTE_VALID;
}

uint32 MemorySetPteReadOnly(uint32 pte){
  return ( (pte & MEM_PTE_MASK) | MEM_PTE_READONLY);
}

void incrementRefTable(uint32 pte){
  uint32 pageNum;
  pageNum = (pte & MEM_PTE_MASK) >> MEM_L1FIELD_FIRST_BITNUM;
  pageRefCounter[pageNum]++;
}

//---------------------------------------------------------------------------
// Handler for handling when a write is attempted on a page that is read-only
//
//---------------------------------------------------------------------------
void MemoryROPViolationHandler(PCB*pcb){
  uint32 culprit_address = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
  uint32 culprit_pageNum = culprit_address >> MEM_L1FIELD_FIRST_BITNUM;
  uint32 i;
  uint32 pageTableIndex;
  uint32 pagenum_alloc;
  
  // Finding pageTableIndex
  i = 0;
  while(pcb->pagetable[i] != culprit_pageNum)
    i++;
  pageTableIndex = i;
  
  // If only one process is using a page, then allow it to be written to and return
  if (pageRefCounter[culprit_pageNum] == 1){
    pcb->pagetable[pageTableIndex] = (culprit_address & MEM_PTE_MASK) | MEM_PTE_VALID;
    return;
  }
  // Copying the entire page to a new page for this process (decrementing old ref page)
  else{
    // Decrementing the ref counter for old page
    pageRefCounter[culprit_pageNum]--;

    // Allocating a new page and copying all the data from the old page here
    pagenum_alloc = MemoryAllocPageEasy(pcb);
    pcb->pagetable[pageTableIndex] = MemorySetupPte(pagenum_alloc);
    MemoryPageCopy(culprit_address, pcb->pagetable[pageTableIndex]);
  }
    return;
}

void MemoryPageCopy(uint32 srcAddress, uint32 destAddress){
  unsigned char* src;
  unsigned char* dest;

  src = srcAddress;
  dest = destAddress;
  
  bcopy(src, dest, MEM_PAGE_SIZE);

  return;
}
//
//
//
void MemoryCopySystemStack(PCB* parentPCB, PCB* childPCB){
  uint32 pagenum_alloc;
  uint32 *stackframe;
  int i;
  uint32 parentBaseAddr;
  uint32 childBaseAddr;
  uint32 parentTopAddr;

  uint32* tmpPtr;

  // Allocating a new page and setting stack frame pointer equal to that
  pagenum_alloc = MemoryAllocPageEasy(childPCB);
  stackframe = (uint32*) ((pagenum_alloc << MEM_L1FIELD_FIRST_BITNUM) | (MEM_PAGE_SIZE - 4));

  // Calculating the base address of the parent system stack (and setting the base of child's
  parentBaseAddr = (uint32)(parentPCB->sysStackPtr) + parentPCB->sysStackArea;
  childBaseAddr =  ((pagenum_alloc << MEM_L1FIELD_FIRST_BITNUM) | (MEM_PAGE_SIZE - 4));

  parentTopAddr = parentBaseAddr - parentPCB->sysStackArea;
    
  // Copying the entire system stack from the parent to the child
  bcopy(&parentBaseAddr, stackframe, parentPCB->stsStackArea);

  // Changing the child sys stack pointer so that it points inside its sys stack page
  childPCB->sysStackPtr = (parentPCB->sysStackPtr - (uint32*)parentBaseAddr) + (uint32)childBaseAddr;
  
  // Iterating through the child sys stack and changing addresses for areas where
  // the address is within the parent's stack
  // starts at base of child stack (top addr) and goes up (to lower addresses)
  tmpPtr = stackframe;
  
  while (tmpPtr != childPCB->sysStackPtr){
    // If the address is within parent sys stack, change it for the child
    if (*tmpPtr <= parentBaseAddr | *tmpPtr >= *(parentPCB->sysStackPtr)){
      *tmpPtr = (*tmpPtr - parentBaseAddr + childBaseAddr);
    }
    tmpPtr -= sizeof(uint32);
  }
  // If the stack pointer address needs to be changed
  if (*tmpPtr <= parentBaseAddr | *tmpPtr >= *(parentPCB->sysStackPtr)){
      *tmpPtr = (*tmpPtr - parentBaseAddr + childBaseAddr);
    }
  // The current stack frame pointer is set to the base of the current interrupt stack frame.
  childPCB->currentSavedFrame = stackframe;
  
}


uint32 malloc(PCB* pcb, int memsize) {
  return MEM_FAIL;
}

uint32 mfree(PCB* pcb, void* mem) {
  return MEM_FAIL;
}
