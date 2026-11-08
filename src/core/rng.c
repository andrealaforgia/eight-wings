#include "core/rng.h"

void rng_seed(Rng* rng, uint32_t seed) {
  rng->state = seed ? seed : 0x9E3779B9u;
  for (int i = 0; i < 4; i++) rng_next(rng);
}

uint32_t rng_next(Rng* rng) {
  uint32_t x = rng->state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng->state = x;
  return x;
}

float rng_float(Rng* rng) { return (float)(rng_next(rng) >> 8) / 16777216.0f; }

int rng_range(Rng* rng, int low, int high) {
  if (high <= low) return low;
  return low + (int)(rng_next(rng) % (uint32_t)(high - low + 1));
}

int rng_chance(Rng* rng, float probability) {
  return rng_float(rng) < probability;
}
