#include <util/rw_lock.h>
#define atomic_xadd(P, V) __sync_fetch_and_add((P), (V))
#define atomic_inc(P) __sync_add_and_fetch((P), 1)

/* Compile read-write barrier */
#define barrier() asm volatile("": : :"memory")

/* Pause instruction to prevent excess processor bus usage */
#define cpu_relax() asm volatile("pause\n": : :"memory")

void write_lock(rwlock_t *l){
    unsigned me = atomic_xadd(&l->u, (1<<16));
    unsigned char val = me >> 16;
    while (val != l->s.write)
        cpu_relax();
}
void write_unlock(rwlock_t *l){
    rwlock_t t = *l;
    barrier();
    t.s.write++;
    t.s.read++;
    *(unsigned short *) l = t.us;
}
void read_lock(rwlock_t *l){
    unsigned me = atomic_xadd(&l->u, (1<<16));
    unsigned char val = me >> 16;
    while (val != l->s.read)
        cpu_relax();
    l->s.read++;
}
void read_unlock(rwlock_t *l){
    atomic_inc(&l->s.write);
}