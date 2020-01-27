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

#ifndef PRK_OMP_H
#define PRK_OMP_H

/* DO NOT INCLUDE ANY C STD HEADERS DIRECTLY IN THIS FILE!
 * Include them in the general header to make sure that
 * certain macros are set before header inclusion. */
#include "par-res-kern_general.h"

#ifdef _OPENMP
#include <omp.h>
#else
#warning Your compiler - probably Clang - does not support OpenMP, so dummy symbols will be used.
/* utility API */
int omp_get_thread_num() { return 0; }
int omp_get_num_threads() { return 1; }
void omp_set_num_threads(int i) { return; }
/* locks API */
typedef int omp_lock_t;
void omp_init_lock(omp_lock_t * l)
{
    *l=0;
}
void omp_destroy_lock(omp_lock_t * l)
{
    /* "It is illegal to call this routine with a lock variable that is not initialized." */
    if (*l==-1) abort();
    *l=-1;
}
void omp_set_lock(omp_lock_t * l)
{
    if (*l==-1) abort();
    if (*l==0) *l=1;
}
void omp_unset_lock(omp_lock_t * l)
{
    if (*l==-1) abort();
    if (*l==1) l=0;
}
int omp_test_lock(omp_lock_t * l)
{
    if (*l==-1) abort();
    if (*l==0) {
        *l=1;
        return 1;
    } else {
        return 0;
    }
}
#endif

#if defined(_OPENMP) && (( __STDC_VERSION__ >= 199901L ) || (__cplusplus >= 201103L ))
#  define PRAGMA(x) _Pragma(#x)
#  if _OPENMP >= 201307
#    define PRAGMA_OMP_FOR_SIMD PRAGMA(omp for simd)
#  else
#    define PRAGMA_OMP_FOR_SIMD PRAGMA(omp for)
#  endif
#else
#  define PRAGMA_OMP_FOR_SIMD
#endif

#ifndef MAXTHREADS
  #define MAX_THREADS 512
#else
  #define MAX_THREADS MAXTHREADS
#endif

// FAASM original:
// extern void bail_out(int);
// FAASM now:
extern void bail_out(int kool) {
    // TODO: panic
    return;
}

#endif /* PRK_OMP_H */
