////////////////////////////////////////////////////////////////////////////////
// Basic Memory Operations

#define LMFENCE asm volatile ("lfence\nmfence\n");
#define MFENCE asm volatile ("mfence\n");
#define LFENCE asm volatile ("lfence\n");

#define READ_ACCESS(x)  ({                                        \
  maccess((void*) (x));                                              })

#define READ(x)  ({                                        \
  mread((void*) (x));                                              })

#define TIME_READ_ACCESS(x)  ({                                   \
  time_mread((void*)x);                             })

#define WRITE_ACCESS(x)  ({                                       \
  memwrite((void*)x);                                             })

#define FLUSH(x)  ({                                              \
  flush((void*)x);                                                })
                                     

////////////////////////////////////////////////////////////////////////////////
// LLC Cache Set Preparation
#define PRIME_EVSETLLC()   ({                              \
  for (i=0; i<PRIME_ITERATIONS; i++) {                                     \
    for (j=0; j<EV_LLC; j++) { READ_ACCESS(evsetLLC[j]); MFENCE; }          \
  }})

#define PRIME_EVSETLLC2()   ({                              \
  for (i=0; i<PRIME_ITERATIONS; i++) {                                     \
    for (j=0; j<EV_LLC; j++) { READ_ACCESS(evsetLLC2[j]); MFENCE; }          \
  }})

#define FLUSH_EVSETLLC() ({ \
  LMFENCE;\
  for (j=0; j<EV_LLC; j++) { FLUSH(evsetLLC[j]); }  \
  LMFENCE;\
})

#define FLUSH_EVSETLLC2() ({ \
  LMFENCE;\
  for (j=0; j<EV_LLC; j++) { FLUSH(evsetLLC2[j]); }  \
  LMFENCE;\
})

#define KILL_HELPER() ({\
  *synchronization=-1;\
  LMFENCE;\
})


 #define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

