#include "readwritelock.h"

void init_rwlock(rwlock_t* rw)
{
    rw->readers = 0;
    sem_init(&(rw->lock),0,1);
    sem_init(&(rw->writelock),0,1);
}

void acquire_readlock(rwlock_t* rw)
{
    sem_wait(&(rw->lock));
    rw->readers++;
    if(rw->readers == 1)
        sem_wait(&(rw->writelock));
    sem_post(&(rw->lock));
}

void release_readlock(rwlock_t* rw)
{
    sem_wait(&(rw->lock));
    rw->readers--;
    if(rw->readers == 0)
        sem_post(&(rw->writelock));
    sem_post(&(rw->lock));
}

void acquire_writelock(rwlock_t* rw)
{
    sem_wait(&(rw->writelock));
}

void release_writelock(rwlock_t* rw)
{
    sem_post(&(rw->writelock));
}
