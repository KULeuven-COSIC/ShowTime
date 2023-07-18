#pragma once

/*
  Ensure that
  attacker and victim (and helper) reside on different physical cores.
  In case of a hyperthreaded machine, logical siblings can be learned, e.g.:
  cat /sys/devices/system/cpu/cpu0/topology/thread_siblings_list
*/
#define ATTACKER_CORE   2
#define HELPER_CORE     0

/* If huge pages are available, indicate it. */
#define HUGE_PAGES_AVAILABLE 0

#define SHARED_MEM_SIZE       (128*MB)
