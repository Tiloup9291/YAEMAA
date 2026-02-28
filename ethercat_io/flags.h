#ifndef _FLAGS_H_
#define _FLAGS_H_

#include <stdatomic.h>
#include <stdint.h>
#include <signal.h>

extern _Atomic uint32_t run_enable;
extern volatile sig_atomic_t shutdown_signal;
#endif /* _FLAGS_H_ */
