////////////////////////////////////////////////////////////////////////////////
// Fences
#define LMFENCE asm volatile ("lfence\nmfence\n");
#define MFENCE asm volatile ("mfence\n");
#define LFENCE asm volatile ("lfence\n");

// Cache manipulation
#define READ_ACCESS(x)  ({                                        \
  maccess((void*) (x));                                              })

#define READ(x)  ({                                        \
  mread((void*) (x));                                              })

#define TIME_READ_ACCESS(x)  ({                                   \
  time_mread((void*)x);                             })

#define WRITE_ACCESS(x)  ({                                       \
  memwrite((void*)x);                                             })

#define WRITE_VALUE(x, v)  ({                                       \
  mwrite_v((void*)(x), v);                                             })

#define FLUSH(x)  ({                                              \
  flush((void*)x);                                                })

////////////////////////////////////////////////////////////////////////////////
                                     
////////////////////////////////////////////////////////////////////////////////
// Occupy L1 with other set
#define EVICT_L1() ({ \
  for (i=0; i<L1_WAYS; i++){ TOUCH(s_evsetL1_c[i]); } \
})

// Memory read with data dependency on j0
#define TOUCH(X) ({ \
  j0 = mread((void*)(X+j0));\
})
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Traversal wrappers
#define MEASURE_PLRU() ({ \
  start = rdtsc();  j0 &= start;\
  for (yit = 0; yit < PLRU_TRAVERSE; yit++){ X_TRAVERSE_NEW(); } \
  LFENCE; rdtsc() - start; \
})

#define MEASURE_PREFETCH(a, b) ({ \
  start = rdtsc();  \
  for (yit = 0; yit < PREFETCH_TRAVERSE; yit++){ prefetchRUN((void*) a, (void*) b); } \
  rdtsc() - start; \
})
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Access pattern to produce (AB-CD)-(EF-GH) PLRU tree
#define X_PLRU_PREPARE_ABC() ({ \
  TOUCH(X_A); TOUCH(X_E); TOUCH(X_C); TOUCH(X_G);\
  TOUCH(X_B); TOUCH(X_F); TOUCH(X_D); TOUCH(X_H);\
})
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Dispatch: select correct traversal pattern depending on macros in settings.h
#if DISTANCE == 1 && REFRESH == 0
  #define X_TRAVERSE_NEW() ({ \
      for (yit = 0; yit < PLRU_TRAVERSE; yit++){ X_TRAVERSE_DISTANCE_1(); } \
  })
#elif DISTANCE == 2 && REFRESH == 0
  #define X_TRAVERSE_NEW() ({ \
      for (yit = 0; yit < PLRU_TRAVERSE; yit++){ X_TRAVERSE_DISTANCE_2(); } \
  })
#elif DISTANCE == 3 && REFRESH == 0
  #define X_TRAVERSE_NEW() ({ \
      for (yit = 0; yit < PLRU_TRAVERSE; yit++){ X_TRAVERSE_DISTANCE_3(); } \
  })
#elif DISTANCE == 1 && REFRESH == 1
  #define X_TRAVERSE_NEW() ({ \
      for (yit = 0; yit < PLRU_TRAVERSE; yit++){ \
          X_TRAVERSE_DISTANCE_1(); \
          if (!(yit & REFRESH_RATE)){ X_REFRESH_DISTANCE_2(); }\
      } \
  })
#elif DISTANCE == 2 && REFRESH == 1
  #define X_TRAVERSE_NEW() ({ \
      for (yit = 0; yit < PLRU_TRAVERSE; yit++){ \
          X_TRAVERSE_DISTANCE_2(); \
          if (!(yit & REFRESH_RATE)){ X_REFRESH_DISTANCE_2(); }\
      } \
  })
#elif DISTANCE == 3 && REFRESH == 1
  #define X_TRAVERSE_NEW() ({ \
      for (yit = 0; yit < PLRU_TRAVERSE; yit++){ \
          X_TRAVERSE_DISTANCE_3(); \
          if (!(yit & REFRESH_RATE)){ X_REFRESH_DISTANCE_3(); }\
      } \
  })
#endif
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Traversal: Assumes (AB-CD)-(EF-GH), and that A was the EVC 
#define X_TRAVERSE_DISTANCE_1() ({ \
  TOUCH(X_A); TOUCH(X_E); TOUCH(X_C); TOUCH(X_B);\
  TOUCH(X_G); TOUCH(X_F); TOUCH(X_D); TOUCH(X_B);\
  TOUCH(X_H); TOUCH(X_A); TOUCH(X_E); TOUCH(X_B);\
  TOUCH(X_C); TOUCH(X_G); TOUCH(X_F); TOUCH(X_B);\
  TOUCH(X_D); TOUCH(X_H); TOUCH(X_A); TOUCH(X_B);\
  TOUCH(X_E); TOUCH(X_C); TOUCH(X_G); TOUCH(X_B);\
  TOUCH(X_F); TOUCH(X_D); TOUCH(X_H); TOUCH(X_B);\
})

#define X_TRAVERSE_DISTANCE_2() ({ \
TOUCH(X_A); TOUCH(X_E); TOUCH(X_B); TOUCH(X_C); TOUCH(X_G); TOUCH(X_B); TOUCH(X_D); TOUCH(X_F); TOUCH(X_B); TOUCH(X_E); TOUCH(X_H); TOUCH(X_B); TOUCH(X_G); TOUCH(X_A); TOUCH(X_B); TOUCH(X_F); TOUCH(X_C); TOUCH(X_B); TOUCH(X_H); TOUCH(X_D); TOUCH(X_B);  \
})

#define X_TRAVERSE_DISTANCE_3() ({ \
TOUCH(X_A); TOUCH(X_B); TOUCH(X_E); TOUCH(X_B); TOUCH(X_G); TOUCH(X_B); TOUCH(X_F); TOUCH(X_B); TOUCH(X_H); TOUCH(X_B); \
})

////////////////////////////////////////////////////////////////////////////////
// Refresh patterns
#define X_REFRESH_DISTANCE_1() ({ \
TOUCH(O_E); TOUCH(O_C); TOUCH(O_G); TOUCH(X_B); TOUCH(O_D); TOUCH(O_F); TOUCH(O_H); TOUCH(X_B); TOUCH(X_E); TOUCH(X_C); TOUCH(X_G); TOUCH(X_B); TOUCH(X_F); TOUCH(X_D); TOUCH(X_H); TOUCH(X_B); \
})

#define X_REFRESH_DISTANCE_2() ({ \
  TOUCH(O_E); TOUCH(O_C); TOUCH(X_B); \
  TOUCH(O_G); TOUCH(O_D); TOUCH(X_B); \
  TOUCH(O_F); TOUCH(X_B); \
  TOUCH(O_H); TOUCH(X_B); \
  TOUCH(X_E); TOUCH(X_C); TOUCH(X_B); TOUCH(X_G); TOUCH(X_D); TOUCH(X_B); TOUCH(X_F); TOUCH(X_B); TOUCH(X_H); TOUCH(X_B); \
})

#define X_REFRESH_DISTANCE_3() ({ \
TOUCH(O_E); TOUCH(X_B); TOUCH(O_G); TOUCH(X_B); TOUCH(O_F); TOUCH(X_B); TOUCH(O_H); TOUCH(X_B); TOUCH(X_E); TOUCH(X_B); TOUCH(X_G); TOUCH(X_B); TOUCH(X_F); TOUCH(X_B); TOUCH(X_H); TOUCH(X_B); \
})

////////////////////////////////////////////////////////////////////////////////
// Adaptor pattern
#define X_TRAVERSE_ORDER_CONVERSION() ({ \
  TOUCH(X_F); TOUCH(X_H); TOUCH(X_B); \
})
