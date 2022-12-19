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

#define LOCK_ASSERT(CONDEXPR, MSG, lock)                       \
    {                                                                          \
        pthread_mutex_unlock(lock);                                            \
        bool should_quit = !(CONDEXPR);                                        \
        if (should_quit)                                                       \
            PANIC(MSG);                                                        \
    }

#define PANIC(MSG)                                                             \
    {                                                                          \
        fprintf(stderr, "Aborting. Reason: %s\n", (MSG));                      \
        abort();                                                               \
    }

#endif
