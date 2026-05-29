//Manages the physical structure of the combinatorial maps

#ifndef GRAPH_H
#define GRAPH_H
#include "common.h"

typedef struct{
    dart_t *twins;
    dart_t *next;
    dart_t *v_head;

    vertex_t *vertices;

    size_t *num_vertices;
    size_t *num_darts;

    void *raw_memory_ptr;
} CACHE_ALIGN CombinatorialGraph;

CombinatorialGraph* graph_allocate(size_t vertices, size_t edges);
void graph_free(CombinatorialGraph *graph);
CombinatorialGraph *parse_input_file(const char *file_path);
#endif
