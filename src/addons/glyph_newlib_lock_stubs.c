#include <sys/lock.h>

struct __lock __lock___sinit_recursive_mutex;
struct __lock __lock___sfp_recursive_mutex;
struct __lock __lock___atexit_recursive_mutex;
struct __lock __lock___at_quick_exit_mutex;
struct __lock __lock___malloc_recursive_mutex;
struct __lock __lock___env_recursive_mutex;
struct __lock __lock___tz_mutex;
struct __lock __lock___dd_hash_mutex;
struct __lock __lock___arc4random_mutex;

void __retarget_lock_init(_LOCK_T *lock) {
    static struct __lock lock_instance;
    if (lock != 0) {
        *lock = &lock_instance;
    }
}

void __retarget_lock_init_recursive(_LOCK_T *lock) {
    __retarget_lock_init(lock);
}

void __retarget_lock_close(_LOCK_T lock) {
    (void)lock;
}

void __retarget_lock_close_recursive(_LOCK_T lock) {
    (void)lock;
}

void __retarget_lock_acquire(_LOCK_T lock) {
    (void)lock;
}

void __retarget_lock_acquire_recursive(_LOCK_T lock) {
    (void)lock;
}

int __retarget_lock_try_acquire(_LOCK_T lock) {
    (void)lock;
    return 1;
}

int __retarget_lock_try_acquire_recursive(_LOCK_T lock) {
    (void)lock;
    return 1;
}

void __retarget_lock_release(_LOCK_T lock) {
    (void)lock;
}

void __retarget_lock_release_recursive(_LOCK_T lock) {
    (void)lock;
}
