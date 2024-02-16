/* ASSERT.H, define the assert macro;
   created by G.Dar in may 1990
   (C) ADPM 1994
   */

#ifndef _ASSERT_DEFINED

#ifndef NDEBUG

static char _assertstring[] = "Assertion failed: %s, file %s, line %d\n";

#define assert(exp) { \
   if(!(exp)) { \
      fprintf(stderr, _assertstring, #exp, __FILE__, __LINE__ );  \
      abort(); \
      } \
   } 

#else

#define assert(exp)

#endif

#define _ASSERT_DEFINED

#endif

      
