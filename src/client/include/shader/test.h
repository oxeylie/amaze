#ifdef __STDC__ // true for a C compiler, not defined by glslang
#include <stdint.h>
typedef uint32_t uint;
#define FN static inline
#else
#define FN
#endif

FN uint twice(uint x) { return x * 2u; }
