#ifndef GIT_HASH
#define GIT_HASH "undef"
#endif

#ifndef GIT_DIRTY
#define GIT_DIRTY "-dirty"
#endif

__attribute__((section(".version"), used)) const char balatro_version[] =
    "GBALATRO_VERSION:" GIT_HASH GIT_DIRTY;
