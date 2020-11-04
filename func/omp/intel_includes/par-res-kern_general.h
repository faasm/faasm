/*
Copyright (c) 2013, Intel Corporation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
      * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PRK_GENERAL_H
#define PRK_GENERAL_H

/* All of this is to get posix_memalign defined... */
/* #define _POSIX_C_SOURCE (200112L) */
#define _POSIX_C_SOURCE (200809L)
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include <inttypes.h>
#include <stdint.h>

#include <unistd.h>

#define PRKVERSION "2.17"
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#endif

#if RESTRICT_KEYWORD
#ifdef __GNUC__
#define RESTRICT __restrict__
#else
#define RESTRICT restrict
#endif
#else
#define RESTRICT
#endif

/* Define 64-bit types and corresponding format strings for printf() */
#ifdef LONG_IS_64BITS
typedef unsigned long u64Int;
typedef long s64Int;
#define FSTR64 "%16ld"
#define FSTR64U "%16lu"
#else
typedef unsigned long long u64Int;
typedef long long s64Int;
#define FSTR64 "%16ll"
#define FSTR64U "%16llu"
#endif

// FAASM: original:
// extern double wtime(void);
// FAASM :
double wtime(void)
{
    // actually return time
    return 1.0;
}

/*  We cannot use C11 aligned_alloc because of this GCC 5.3.0 bug:
 *  https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69680 */
#if 0 && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define PRK_HAS_C11 1
#endif

/* This function is separate from prk_malloc() because
 * we need it when calling prk_shmem_align(..)           */
static inline int prk_get_alignment(void)
{
    /* a := alignment */
#ifdef PRK_ALIGNMENT
    int a = PRK_ALIGNMENT;
#else
    char* temp = getenv("PRK_ALIGNMENT");
    int a = (temp != NULL) ? atoi(temp) : 64;
    if (a < 8)
        a = 8;
    assert((a & (~a + 1)) == a);
#endif
    return a;
}

/* There are a variety of reasons why this function is not declared by stdlib.h.
 */
#if defined(__UPC__)
int posix_memalign(void** memptr, size_t alignment, size_t size);
#endif

static inline void* prk_malloc(size_t bytes)
{
#ifndef PRK_USE_MALLOC
    int alignment = prk_get_alignment();
#endif

/* Berkeley UPC throws warnings related to this function for no obvious
 * reason... */
#if !defined(__UPC__) && defined(__INTEL_COMPILER) &&                          \
  !defined(PRK_USE_POSIX_MEMALIGN)
    return (void*)_mm_malloc(bytes, alignment);
#elif defined(PRK_HAS_C11)
    /* From ISO C11:
     *
     * "The aligned_alloc function allocates space for an object
     *  whose alignment is specified by alignment, whose size is
     *  specified by size, and whose value is indeterminate.
     *  The value of alignment shall be a valid alignment supported
     *  by the implementation and the value of size shall be an
     *  integral multiple of alignment."
     *
     *  Thus, if we do not round up the bytes to be a multiple
     *  of the alignment, we violate ISO C.
     */
    size_t padded = bytes;
    size_t excess = bytes % alignment;
    if (excess > 0)
        padded += (alignment - excess);
    return aligned_alloc(alignment, padded);
#elif defined(PRK_USE_MALLOC)
#warning PRK_USE_MALLOC prevents the use of alignmed memory.
    return prk_malloc(bytes);
#else /* if defined(PRK_USE_POSIX_MEMALIGN) */
    void* ptr = NULL;
    int ret;
    ret = posix_memalign(&ptr, alignment, bytes);
    if (ret)
        ptr = NULL;
    return ptr;
#endif
}

static inline void prk_free(void* p)
{
#if !defined(__UPC__) && defined(__INTEL_COMPILER) &&                          \
  !defined(PRK_USE_POSIX_MEMALIGN)
    _mm_free(p);
#else
    free(p);
#endif
}

/* dt is the time interval in seconds that you want to waste */
static inline void prk_pause(double dt)
{
    double t0 = wtime();
    while (1) {
        double t1 = wtime();
        if ((t1 - t0) > dt)
            return;
    }
    return;
}

/* find factorization of an integer into two factors that are as close together
   as possible. If not the same, second factor is the largest */
static inline void factor(int r, int* fac1, int* fac2)
{
    for (*fac1 = (int)(sqrt(r + 1.0)); *fac1 > 0; (*fac1)--) {
        if (!(r % (*fac1))) {
            *fac2 = r / (*fac1);
            break;
        }
    }
}

/* find optimal division of a pair of integers whose sum is fixed, such that the
   pair is as close as possible to an input pair, but whose two-term
   factorizations have minimal maximum aspect ratios */
static inline void optimize_split(int* r_in0, int* r_in1, float lb_weight)
{
    int r[2], fac[2][2], delta, r_opt[2], i;
    float max_aspect, imbalance, cost, cost_opt;

    cost_opt = (float)INT64_MAX;
    for (delta = -MIN((*r_in0), (*r_in1)) + 1; delta < MIN((*r_in0), (*r_in1));
         delta++) {
        r[0] = (*r_in0 - delta);
        r[1] = (*r_in1) + delta;
        for (i = 0; i < 2; i++)
            factor(r[i], &fac[i][0], &fac[i][1]);
        max_aspect = 1.0 - 1.0 / MAX((float)fac[0][1] / (float)fac[0][0],
                                     (float)fac[1][1] / (float)fac[1][0]);
        imbalance = MAX(1.0 - (float)r[1] / (float)(*r_in1),
                        1.0 - (float)r[0] / (float)(*r_in0));
        cost = imbalance * lb_weight + max_aspect;
        if (cost < cost_opt) {
            cost_opt = cost;
            for (i = 0; i < 2; i++)
                r_opt[i] = r[i];
        }
    }
    *r_in0 = r_opt[0];
    *r_in1 = r_opt[1];
}

#endif /* PRK_GENERAL_H */
