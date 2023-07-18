////////////////////////////////////////////////////////////////////////////////
// Assign names to cache lines
#if AMPLIFIER_LLC_CONGRUENT == 1
  #define X_A LLC_LOCATION[0]
  #define X_B LLC_LOCATION[5]
  #define X_C LLC_LOCATION[2]
  #define X_D LLC_LOCATION[4]
  #define X_E LLC_LOCATION[1]
  #define X_F LLC_LOCATION[6]
  #define X_G LLC_LOCATION[7]
  #define X_H LLC_LOCATION[3]
#else
  #define X_A LLC_LOCATION[0]
  #define X_B L1a_LOCATION[5]
  #define X_C L1a_LOCATION[2]
  #define X_D L1a_LOCATION[4]
  #define X_E L1a_LOCATION[1]
  #define X_F L1a_LOCATION[6]
  #define X_G L1a_LOCATION[7]
  #define X_H L1a_LOCATION[3]
#endif

#define X_X  L1b_LOCATION[0]
#define X_Y  L1b_LOCATION[1]
#define X_PS LLC2_LOCATION[2]

#define M_A (evsetLLC2_c[0])
#define M_B (evsetLLC2_c[1])
#define M_C (evsetLLC2_c[2])
#define M_D (evsetLLC2_c[3])
#define M_E (evsetLLC2_c[4])

// For refresh patterns
#define O_E (L1c_LOCATION[1])
#define O_C (L1c_LOCATION[2])
#define O_G (L1c_LOCATION[3])
#define O_D (L1c_LOCATION[4])
#define O_F (L1c_LOCATION[5])
#define O_H (L1c_LOCATION[6])
#define O_A (L1c_LOCATION[7])
#define O_B X_B

#define X_T      VAR_LOCATION[6]
#define X_T2     VAR_LOCATION[3]
#define X_Z      VAR_LOCATION[1]
#define ITERATOR VAR_LOCATION[0]
#define YIT VAR_LOCATION[4]
////////////////////////////////////////////////////////////////////////////////
