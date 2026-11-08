/**
 * @file rng.h
 * @brief Small deterministic xorshift32 generator. A seed always gives the same
 * level, the same fight and the same test run. Nothing here is global.
 *
 * Adapted from super-fabio-bros/engine/core/math/rng.h (MIT, (c) 2026 Andrea
 * Laforgia); see THIRD_PARTY.md.
 */
#ifndef CORE_RNG_H_
#define CORE_RNG_H_

#include <stdint.h>

typedef struct {
  uint32_t state;
} Rng;

void rng_seed(Rng* rng, uint32_t seed);
uint32_t rng_next(Rng* rng);
/** In [0, 1). */
float rng_float(Rng* rng);
/** In [low, high], both included. */
int rng_range(Rng* rng, int low, int high);
/** Non-zero with the given probability. */
int rng_chance(Rng* rng, float probability);

#endif  // CORE_RNG_H_
