#include <stdint.h>
#include <stdlib.h>

extern volatile uint64_t *synchronization;

void attacker_helper() {

  while(1) {

    if (*synchronization == -1) {
      break;
    }
   
  }

  exit(EXIT_SUCCESS);
}