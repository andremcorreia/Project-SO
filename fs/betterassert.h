#ifndef _BETTER_ASSERT_H
#define _BETTER_ASSERT_H

#include <stdbool.h>
#include <stdio.h>

#define ALWAYS_ASSERT(CONDEXPR, MSG)                                           \
    {                                                                          \
        bool should_quit = !(CONDEXPR);                                        \
        if (should_quit)                                                       \
            PANIC(MSG);                                                        \
    }

#define LOCK_ASSERT(CONDEXPR, MSG, lock)                                       \
    {                                                                                                                     \
        bool should_quit = !(CONDEXPR);                                        \
        if (should_quit) {                                                     \
            pthread_mutex_unlock(lock);                                        \
            PANIC(MSG);                                                        \
        }                                                                      \
    }
    
#define LOCK_ASSERT_MUTEX_RW(CONDEXPR, MSG, lock, rw_lock)                     \
    {                                                                          \
        bool should_quit = !(CONDEXPR);                                        \
        if (should_quit) {                                                     \
            pthread_mutex_unlock(lock);                                        \
            pthread_rwlock_unlock(rw_lock);                                    \
            PANIC(MSG);                                                        \
        }}                                                                      
    

#define LOCK_ASSERT_RW_MUTEX(CONDEXPR, MSG, rw_lock, lock)                     \
    {                                                                          \
        bool should_quit = !(CONDEXPR);                                        \
        if (should_quit) {                                                     \
            pthread_rwlock_unlock(rw_lock);                                    \
            pthread_mutex_unlock(lock);                                        \
            PANIC(MSG);                                                        \
        }}                                                                      

#define LOCK_ASSERT_RW(CONDEXPR, MSG, rw_lock)                                 \
    {                                                                          \
        bool should_quit = !(CONDEXPR);                                        \
        if (should_quit) {                                                     \
            pthread_rwlock_unlock(rw_lock);                                    \
            PANIC(MSG);                                                        \
        }}                                                                      
    

#define LOCK_ASSERT_RW_RW(CONDEXPR, MSG, rw_lock,rw_lock2)                     \
    {                                                                          \
        bool should_quit = !(CONDEXPR);                                        \
        if (should_quit) {                                                     \
            pthread_rwlock_unlock(rw_lock);                                    \
            pthread_rwlock_unlock(rw_lock2);                                   \
            PANIC(MSG);                                                        \
        }}                                                                      
    

#define PANIC(MSG)                                                             \
    {                                                                          \
        fprintf(stderr, "Aborting. Reason: %s\n", (MSG));                      \
        abort();                                                               \
    }

#endif
