#include "../include/graph.h"
#include "../include/simd_match.h"
#include "../include/reduction.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    // 1. Ingest files from parameters (e.g., parsing raw adjacency descriptions)
    CombinatorialGraph *g1 = parse_input_file(argv[1]);
    CombinatorialGraph *g2 = parse_input_file(argv[2]);

    // 2. Immediate topological verification
    if (g1->num_vertices != g2->num_vertices || g1->num_darts != g2->num_darts) {
        printf("Graphs have mismatched dimensional layouts. Non-isomorphic.\n");
        return 0;
    }

    // 3. Fast Vector Check
    if (!simd_fast_reject(g1, g2)) {
        printf("SIMD step: Vertex structural invariants mismatch. Non-isomorphic.\n");
        return 0;
    }

    // 4. Run full reduction pipeline
    CanonicalStream *code1 = graph_reduce_to_code(g1);
    CanonicalStream *code2 = graph_reduce_to_code(g2);

    // 5. Final Isomorphism Evaluation
    if (code1->head == code2->head && memcmp(code1->buffer, code2->buffer, code1->head) == 0) {
        printf("Isomorphism verified successfully.\n");
    } else {
        printf("Graphs are distinct (reduction signatures mismatched).\n");
    }

    graph_free(g1);
    graph_free(g2);
    return 0;
}
