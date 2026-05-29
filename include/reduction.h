// include/reduction.h
#ifndef REDUCTION_H
#define REDUCTION_H
#include "graph.h"

typedef struct {
    uint8_t *buffer;
    size_t head;
    size_t capacity;
} CanonicalStream;

// The main loop that reduces a graph to its base canonical form string
CanonicalStream* graph_reduce_to_code(CombinatorialGraph *g);

#endif
