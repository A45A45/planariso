#include "../include/simd_match.h"
#include <../include/immintrin.h>
#include <string.h>
#include <stdlib.h>

// Static internal helper to compute signatures for a graph
static int64_t* compute_vertex_signatures(CombinatorialGraph *g) {
    // Ensure allocation is aligned to a 32-byte boundary for AVX2 loading
    int64_t *signatures = _mm_malloc(*g->num_vertices * sizeof(int64_t), 32);
    if (!signatures) return NULL;

    for (size_t v = 0; v < *g->num_vertices; v++) {
        dart_t start_dart = g->v_head[v];
        if (start_dart == NULL_DART) {
            signatures[v] = 0;
            continue;
        }

        int32_t degree = 0;
        int32_t neighbor_degree_sum = 0;
        dart_t curr = start_dart;

        // Traverse the clockwise orbit of darts around vertex v
        do {
            degree++;
            vertex_t neighbor = g->vertices[curr ^ 1]; // Twin dart's origin

            // Quickly calculate neighbor's degree by scanning its orbit
            dart_t n_curr = g->v_head[neighbor];
            int32_t n_deg = 0;
            if (n_curr != NULL_DART) {
                do { n_deg++; n_curr = g->next[n_curr]; } while (n_curr != g->v_head[neighbor]);
            }
            neighbor_degree_sum += n_deg;

            curr = g->next[curr];
        } while (curr != start_dart);

        // Pack invariants: lower 32 bits = degree, upper 32 bits = neighbor structural weight
        signatures[v] = ((int64_t)neighbor_degree_sum << 32) | (degree & 0xFFFFFFFF);
    }
    return signatures;
}

static int compare_i64(const void *a, const void *b) {
    int64_t x = *(const int64_t *)a;
    int64_t y = *(const int64_t *)b;
    return (x > y) - (x < y);  // branchless, avoids overflow vs subtraction
}

int simd_fast_reject(CombinatorialGraph *g1, CombinatorialGraph *g2) {
 int64_t *sigs1 = compute_vertex_signatures(g1);
    int64_t *sigs2 = compute_vertex_signatures(g2);

    if (!sigs1 || !sigs2) {
        if (sigs1) _mm_free(sigs1);
        if (sigs2) _mm_free(sigs2);
        return 0; // Allocation failure defaults to safety reject
    }

    // Sort signatures so we can check for an exact structural multi-set match
    // (A quick standard qsort works here since this is a flat array)
    qsort(sigs1, *g1->num_vertices, sizeof(int64_t), compare_i64);
    qsort(sigs2, *g2->num_vertices, sizeof(int64_t), compare_i64);

    int match = 1;
    size_t i = 0;
    size_t rounded_count = *g1->num_vertices & ~3UL; // Round down to multiple of 4

    // AVX2 Main Loop - Vectorized comparison step
    for (; i < rounded_count; i += 4) {
        __m256i vec1 = _mm256_load_si256((__m256i*)&sigs1[i]);
        __m256i vec2 = _mm256_load_si256((__m256i*)&sigs2[i]);

        __m256i cmp = _mm256_cmpeq_epi64(vec1, vec2);
        int mask = _mm256_movemask_epi8(cmp);

        if (mask != 0xFFFFFFFF) { // Mismatch found inside this 4-element block
            match = 0;
            break;
        }
    }

    // Clean up any remaining clean scalar trail elements (if V % 4 != 0)
    if (match) {
        for (; i < *g1->num_vertices; i++) {
            if (sigs1[i] != sigs2[i]) {
                match = 0;
                break;
            }
        }
    }

    _mm_free(sigs1);
    _mm_free(sigs2);
    return match;
}
