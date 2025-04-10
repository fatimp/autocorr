#ifndef __PARAM_INIT_H__
#define __PARAM_INIT_H__

#include <stdint.h>

#define P 23068673
#define MAX_STEPS 21

extern uint64_t _omegas[MAX_STEPS + 1];
extern uint64_t _inv_omegas[MAX_STEPS + 1];
extern uint64_t _inv_lengths[MAX_STEPS + 1];

#endif
