////////////////////////////////////////////////////////////////////////////////
#define LMFENCE asm volatile ("lfence\nmfence\n");
#define MFENCE asm volatile ("mfence\n");
#define LFENCE asm volatile ("lfence\n");
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Basic Memory Operations

#define READ_ACCESS(x)  ({                                        \
  maccess((void*)x);                                              })

#define TIME_READ_ACCESS(x)  ({                                   \
  time_mread((void*)x);                             })

#define WRITE_ACCESS(x)  ({                                       \
  memwrite((void*)x);                                             })

#define FLUSH(x)  ({                                              \
  flush((void*)x);                                                })


////////////////////////////////////////////////////////////////////////////////
// Memory Operations to be executed by the helper thread

#define HELPER_READ_ACCESS(x)   ({                                \
  *synchronization_params = (volatile uint64_t)x;                  \
  *synchronization = 1;                                            \
  while(*synchronization==1);                                      })

#define HELPER_WRITE()   ({                                \
  *synchronization = 3; })
  //while(*synchronization==1);                                      })

#define HELPER_STAMP()   ({                                \
  *synchronization = 4; })

#define KILL_HELPER()   ({                                \
  *synchronization = 99;                                            \
  while(*synchronization==99);                                      })

#define HELPER_FLUSH(x)   ({                                \
  *synchronization_params = (volatile uint64_t)x;                  \
  *synchronization = 2;                                            \
  while(*synchronization==2);                                      })

#define HELPER_EVICT_LLC(n)   ({                                \
  *synchronization_params = (volatile uint64_t)n;                  \
  *synchronization = 20;                                            \
  while(*synchronization==20);                                      })


////////////////////////////////////////////////////////////////////////////////
// Memory Operations to be executed by the victim thread

#define VICTIM_READ_ACCESS(x)   ({                                \
  *synchronization_params = (volatile uint64_t)x;                  \
  *synchronization = 11;                                           \
  while(*synchronization==11);                                     })

////////////////////////////////////////////////////////////////////////////////
// Extras

#define BUSY_WAIT() ({                                            \
  for (i = 30000; i>0; i--)                                       \
    asm volatile("nop;");                                         })
