/*Architecture types, bit-masks, macro assertions and global optimizations.*/

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>

//Force cache alignment for standart CPU cache boundaries
#define CACHE_ALIGN __attribute__((aligned(64)))

typedef int32_t dart_t ;
typedef int32_t vertex_t;

#define NULL_DART (-1)

#endif
