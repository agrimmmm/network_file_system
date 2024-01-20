#ifndef __READ_WRITE_LOCK_H__
#define __READ_WRITE_LOCK_H__

#include <semaphore.h>

typedef struct rwlock_t
{
    int readers;
    sem_t lock;
    sem_t writelock;
}rwlock_t;

void init_rwlock(rwlock_t* rw);
void acquire_readlock(rwlock_t *rw);
void release_readlock(rwlock_t *rw);
void acquire_writelock(rwlock_t *rw);
void release_writelock(rwlock_t *rw);


#endif