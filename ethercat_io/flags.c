#include "flags.h"

_Atomic uint32_t run_enable __attribute__((aligned(64))) = 0;
volatile sig_atomic_t shutdown_signal = 0;
