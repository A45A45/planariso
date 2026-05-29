#include "../include/graph.h"
#include <stddef.h>
#include <sys/mman.h>
#include <stdlib.h>

CombinatorialGraph* graph_allocate(size_t vertices, size_t edges){
    size_t darts = edges * 2;

        // Calculate total layout bounds for memory compaction
        size_t size_twins    = darts * sizeof(dart_t);
        size_t size_next     = darts * sizeof(dart_t);
        size_t size_vertices = darts * sizeof(vertex_t);
        size_t size_v_head   = vertices * sizeof(dart_t);
        size_t total_alloc   = size_twins + size_next + size_vertices + size_v_head + sizeof(CombinatorialGraph);

        // Allocate memory anonymously using huge pages to bypass OS translation lookaside buffer misses
        void *mem = mmap(NULL, total_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        if (mem == MAP_FAILED) {
            // Fallback if Gentoo kernel doesn't have hugepages allocated
            mem = mmap(NULL, total_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        }

        CombinatorialGraph *g = (CombinatorialGraph*)mem;
            uint8_t *offset = (uint8_t*)mem + sizeof(CombinatorialGraph);

            g->twins = (dart_t*)offset;     offset += size_twins;
            g->next = (dart_t*)offset;      offset += size_next;
            g->vertices = (vertex_t*)offset; offset += size_vertices;
            g->v_head = (dart_t*)offset;

            g->num_vertices = &vertices;
            g->num_darts = &darts;
            g->raw_memory_ptr = mem;

            return g;
}
