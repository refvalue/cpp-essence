include_guard()

set(c_cxx_linker_flags "-Wl,--wrap=__libc_single_threaded,--wrap=pthread_create,--wrap=pthread_join,--wrap=pthread_detach,--wrap=pthread_once,--wrap=pthread_getspecific,--wrap=pthread_setspecific,--wrap=pthread_key_create,--wrap=pthread_key_delete,--wrap=pthread_rwlock_init,--wrap=pthread_rwlock_rdlock,--wrap=pthread_rwlock_wrlock,--wrap=pthread_rwlock_unlock,--wrap=pthread_rwlock_destroy,--wrap=pthread_sigmask,--wrap=pthread_condattr_setclock,--wrap=pthread_cond_clockwait,--wrap=dlopen,--wrap=dlclose,--wrap=dladdr,--wrap=dlsym,--wrap=dlerror,--wrap=stat,--wrap=fstat,--wrap=stat64,--wrap=fstat64,--wrap=fcntl")
string(APPEND CMAKE_C_FLAGS " ${c_cxx_linker_flags}")
string(APPEND CMAKE_CXX_FLAGS " ${c_cxx_linker_flags}")
unset(c_cxx_linker_flags)
