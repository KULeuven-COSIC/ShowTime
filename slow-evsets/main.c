#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <getopt.h>
#include <assert.h>
#define ASSERT(x) assert(x != -1)

#include "../utils/configuration.h"
#include "../utils/memory_utils.h"
#include "settings.h"
#include "../utils/misc_utils.h"

////////////////////////////////////////////////////////////////////////////////
// Function declarations
void attacker();

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv){

  set_core(ATTACKER_CORE, "Attacker"); 
  attacker();

  return 0;
}