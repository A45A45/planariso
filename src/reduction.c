#include "../include/reduction.h"
#include <stdlib.h>
#include <string.h>

static void append_to_stream(CanonicalStream *stream, uint8_t value) {
    if (stream->head >= stream->capacity) {
        stream->capacity *= 2;
        stream->buffer = realloc(stream->buffer, stream->capacity);
    }
    stream->buffer[stream->head++] = value;
}

// In-place elimination of a vertex from the flat combinatorial map layout
static void isolate_and_remove_vertex(CombinatorialGraph *g, vertex_t v, CanonicalStream *stream) {
    dart_t start_dart = g->v_head[v];
    dart_t curr = start_dart;

    // Log the vertex degree to the stream before destruction
    uint8_t deg = 0;
    do { deg++; curr = g->next[curr]; } while(curr != start_dart);
    append_to_stream(stream, deg);

    // Bypass this vertex entirely by updating the combinatorial links of its neighbors
    curr = start_dart;
    do {
        dart_t twin = curr ^ 1;
        vertex_t neighbor = g->vertices[twin];

        // Find the dart leading back to v from the neighbor's perspective
        dart_t n_curr = g->v_head[neighbor];
        dart_t n_prev = NULL_DART;

        // Iterate through neighbor's local rotation orbit
        do {
            if (g->vertices[n_curr ^ 1] == v) {
                // Bridge the gap over the deleted connection
                if (n_prev == NULL_DART) {
                    // We are at the head of the neighbor's orbit list
                    dart_t find_last = g->v_head[neighbor];
                    while (g->next[find_last] != n_curr) {
                        find_last = g->next[find_last];
                    }
                    g->next[find_last] = g->next[n_curr];
                    g->v_head[neighbor] = g->next[n_curr];
                } else {
                    g->next[n_prev] = g->next[n_curr];
                }
                break;
            }
            n_prev = n_curr;
            n_curr = g->next[n_curr];
        } while (n_curr != g->v_head[neighbor]);

        curr = g->next[curr];
    } while (curr != start_dart);

    // Completely clear out the target head index tracking pointer
    g->v_head[v] = NULL_DART;
}

CanonicalStream* graph_reduce_to_code(CombinatorialGraph *g) {
    CanonicalStream *stream = malloc(sizeof(CanonicalStream));
    stream->capacity = 1024;
    stream->head = 0;
    stream->buffer = malloc(stream->capacity);

    // Keep track of which vertices have already been eliminated
    uint8_t *removed = calloc(*g->num_vertices, sizeof(uint8_t));

    size_t vertices_remaining = *g->num_vertices;
    while (vertices_remaining > 0) {
        vertex_t target_vertex = -1;

        // Linear scan for an available vertex of degree <= 5
        for (size_t v = 0; v < *g->num_vertices; v++) {
            if (g->v_head[v] == NULL_DART || removed[v]) continue;

            // Compute current active degree
            int32_t current_deg = 0;
            dart_t start = g->v_head[v];
            dart_t curr = start;
            do {
                current_deg++;
                curr = g->next[curr];
            } while (curr != start);

            if (current_deg <= 5) {
                target_vertex = v;
                break;
            }
        }

        // If no vertex has degree <= 5, this is not a valid planar graph representation
        if (target_vertex == -1) {
            // Log emergency break sequence code
            append_to_stream(stream, 0xFF);
            break;
        }

        // Deconstruct the chosen vertex and write its topology fingerprint to the stream
        isolate_and_remove_vertex(g, target_vertex, stream);
        removed[target_vertex] = 1;
        vertices_remaining--;
    }

    free(removed);
    return stream;
}
