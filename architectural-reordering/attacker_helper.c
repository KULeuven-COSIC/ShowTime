#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../utils/cache_utils.h"
#include "settings.h"

#define NOP asm volatile ("nop\n");
#define NOP4 asm volatile ("nop\nnop\nnop\nnop");
#define NOP8 NOP4; NOP4;
#define NOP16 NOP8; NOP8;

extern volatile uint64_t *shared_mem;
extern volatile uint64_t *synchronization;
extern volatile uint64_t *synchronization_params;

void attacker_helper() {

  //////////////////////////////////////////////////////////////////////////////
  // Prepare variables for test cache access times

  #define FENCE asm volatile ("mfence\n\t lfence\n\t");

  //////////////////////////////////////////////////////////////////////////////

  int r;
  int i = 0;

  while(1) {

    if (*synchronization == -1) {
      break;
    }
    if (*synchronization == 99) {
      /* Implements the KILL_HELPER() macro */
      *synchronization = 0;
      break;
    }
    else if (*synchronization == 1) {
      /* Implements the HELPER_READ_ACCESS() macro */
      memread((void*)*synchronization_params);
      FENCE
      *synchronization = 0;
    }
    else if (*synchronization == 2) {
      /* Implements the HELPER_FLUSH() macro */
      flush((void*)*synchronization_params);
      FENCE
      *synchronization = 0;
    }
    else if (*synchronization == 3) {
      /* Implements the HELPER_WRITE() macro */
      NOP4;
      shared_mem[0] = 0xFFFFFFFF;
      FENCE
      *synchronization = 0;
    }
    else if (*synchronization == 4) {
      shared_mem[i++] = rdtscp64();
      asm volatile("mfence");
      *synchronization = 0;
    }
  }

  exit(EXIT_SUCCESS);
}
