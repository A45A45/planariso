// include/simd_match.h
#ifndef SIMD_MATCH_H
#define SIMD_MATCH_H
#include "graph.h"

// Computes signatures and runs immediate AVX vector comparisons
int simd_fast_reject(CombinatorialGraph *g1, CombinatorialGraph *g2);

#endif
