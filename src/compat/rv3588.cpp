/*
 * Copyright (c) 2024 The RefValue Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdarg>

#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
char __wrap___libc_single_threaded = false;

// https://refspecs.linuxbase.org/LSB_3.0.0/LSB-PDA/LSB-PDA/baselib-xstat-1.html
int __xstat(int ver, const char* path, struct stat* stat_buf);
int __fxstat(int ver, int fildes, struct stat* stat_buf);

// https://refspecs.linuxbase.org/LSB_3.0.0/LSB-PDA/LSB-PDA/baselib-xstat64-1.html
int __xstat64(int ver, const char* path, struct stat64* stat_buf);
int __fxstat64(int ver, int fildes, struct stat64* stat_buf);

int __libc_start_main(int*(main) (int, char**, char**), int argc, char** ubp_av, void (*init)(void), void (*fini)(void),
    void (*rtld_fini)(void), void(*stack_end));

int __wrap___libc_start_main(int*(main) (int, char**, char**), int argc, char** ubp_av, void (*init)(void),
    void (*fini)(void), void (*rtld_fini)(void), void(*stack_end)) {
    asm(".symver __libc_start_main, __libc_start_main@GLIBC_2.17");
    return __libc_start_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}

int __wrap_pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg) {
    asm(".symver pthread_create, pthread_create@GLIBC_2.17");
    return pthread_create(thread, attr, start_routine, arg);
}

int __wrap_pthread_join(pthread_t thread, void** retval) {
    asm(".symver pthread_join, pthread_join@GLIBC_2.17");
    return pthread_join(thread, retval);
}

int __wrap_pthread_detach(pthread_t thread) {
    asm(".symver pthread_detach, pthread_detach@GLIBC_2.17");
    return pthread_detach(thread);
}

int __wrap_pthread_once(pthread_once_t* once_control, void (*init_routine)(void)) {
    asm(".symver pthread_once, pthread_once@GLIBC_2.17");
    return pthread_once(once_control, init_routine);
}

void* __wrap_pthread_getspecific(pthread_key_t key) {
    asm(".symver pthread_getspecific, pthread_getspecific@GLIBC_2.17");
    return pthread_getspecific(key);
}

int __wrap_pthread_setspecific(pthread_key_t key, const void* value) {
    asm(".symver pthread_setspecific, pthread_setspecific@GLIBC_2.17");
    return pthread_setspecific(key, value);
}

int ____wrap_pthread_key_create(pthread_key_t* key, void (*destructor)(void*)) {
    asm(".symver __pthread_key_create, __pthread_key_create@GLIBC_2.17");
    return pthread_key_create(key, destructor);
}

int __wrap_pthread_key_create(pthread_key_t* key, void (*destructor)(void*)) {
    asm(".symver pthread_key_create, pthread_key_create@GLIBC_2.17");
    return pthread_key_create(key, destructor);
}

int __wrap_pthread_key_delete(pthread_key_t key) {
    asm(".symver pthread_key_delete, pthread_key_delete@GLIBC_2.17");
    return pthread_key_delete(key);
}

int __wrap_pthread_rwlock_init(pthread_rwlock_t* rwlock, const pthread_rwlockattr_t* attr) {
    asm(".symver pthread_rwlock_init, pthread_rwlock_init@GLIBC_2.17");
    return pthread_rwlock_init(rwlock, attr);
}

int __wrap_pthread_rwlock_rdlock(pthread_rwlock_t* rwlock) {
    asm(".symver pthread_rwlock_rdlock, pthread_rwlock_rdlock@GLIBC_2.17");
    return pthread_rwlock_rdlock(rwlock);
}

int __wrap_pthread_rwlock_wrlock(pthread_rwlock_t* rwlock) {
    asm(".symver pthread_rwlock_wrlock, pthread_rwlock_wrlock@GLIBC_2.17");
    return pthread_rwlock_wrlock(rwlock);
}

int __wrap_pthread_rwlock_unlock(pthread_rwlock_t* rwlock) {
    asm(".symver pthread_rwlock_unlock, pthread_rwlock_unlock@GLIBC_2.17");
    return pthread_rwlock_unlock(rwlock);
}

int __wrap_pthread_rwlock_destroy(pthread_rwlock_t* rwlock) {
    asm(".symver pthread_rwlock_destroy, pthread_rwlock_destroy@GLIBC_2.17");
    return pthread_rwlock_destroy(rwlock);
}

int __wrap_pthread_sigmask(int how, const sigset_t* set, sigset_t* oldset) {
    asm(".symver pthread_sigmask, pthread_sigmask@GLIBC_2.17");
    return pthread_sigmask(how, set, oldset);
}

int __wrap_pthread_condattr_setclock(pthread_condattr_t* attr, clockid_t clock_id) {
    asm(".symver pthread_condattr_setclock, pthread_condattr_setclock@GLIBC_2.17");
    return pthread_condattr_setclock(attr, clock_id);
}

int __wrap_pthread_cond_clockwait(
    pthread_cond_t* cond, pthread_mutex_t* mutex, clockid_t clock_id, const struct timespec* abstime) {
    asm(".symver pthread_cond_timedwait, pthread_cond_timedwait@GLIBC_2.17");
    return pthread_cond_timedwait(cond, mutex, abstime);
}

void* __wrap_dlopen(const char* filename, int flags) {
    asm(".symver dlopen, dlopen@GLIBC_2.17");
    return dlopen(filename, flags);
}

int __wrap_dlclose(void* handle) {
    asm(".symver dlclose, dlclose@GLIBC_2.17");
    return dlclose(handle);
}

int __wrap_dladdr(const void* addr, Dl_info* info) {
    asm(".symver dladdr, dladdr@GLIBC_2.17");
    return dladdr(addr, info);
}

void* __wrap_dlsym(void* handle, const char* symbol) {
    asm(".symver dlsym, dlsym@GLIBC_2.17");
    return dlsym(handle, symbol);
}

char* __wrap_dlerror(void) {
    asm(".symver dlerror, dlerror@GLIBC_2.17");
    return dlerror();
}

int __wrap_stat(const char* path, struct stat* buf) {
    asm(".symver __xstat, __xstat@GLIBC_2.17");
    return __xstat(3, path, buf);
}

int __wrap_fstat(int fd, struct stat* buf) {
    asm(".symver __fxstat, __fxstat@GLIBC_2.17");
    return __fxstat(3, fd, buf);
}

int __wrap_stat64(const char* path, struct stat64* buf) {
    asm(".symver __xstat64, __xstat64@GLIBC_2.17");
    return __xstat64(3, path, buf);
}

int __wrap_fstat64(int fd, struct stat64* buf) {
    asm(".symver __fxstat64, __fxstat64@GLIBC_2.17");
    return __fxstat64(3, fd, buf);
}
}
