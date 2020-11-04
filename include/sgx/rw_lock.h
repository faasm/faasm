#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    typedef union __rwlock
    {
        unsigned u;
        unsigned short us;
        __extension__ struct
        {
            unsigned char write;
            unsigned char read;
            unsigned char users;
        } s;
    } rwlock_t;

    typedef void (*unlock_func)(rwlock_t*);

    void write_lock(rwlock_t* l);

    void write_unlock(rwlock_t* l);

    void read_lock(rwlock_t* l);

    void read_unlock(rwlock_t* l);

#ifdef __cplusplus
}
#endif
