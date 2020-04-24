#ifndef _AVERISERA_F2C_TYPES_H
#define _AVERISERA_F2C_TYPES_H

#ifndef F2C_INCLUDE

/* Type definitions for f2c.h, factored out to allow declaring functions using f2c without pulling in dodgy macros */

#if defined(_WIN64) || defined(__alpha__) || defined(__sparc64__) || defined(__x86_64__) || defined(__ia64__)
typedef int integer;
typedef unsigned int uinteger;
#else
typedef long int integer;
typedef unsigned long int uinteger;
#endif
typedef char *address;
typedef short int shortint;
//typedef float real;
typedef double doublereal;
//typedef struct { real r, i; } complex;
typedef struct { doublereal r, i; } doublecomplex;
#if defined(_WIN64) || defined(__alpha__) || defined(__sparc64__) || defined(__x86_64__) || defined(__ia64__)
typedef int logical;
#else
typedef long int logical;
#endif
typedef short int shortlogical;
typedef char logical1;
typedef char integer1;
#ifdef INTEGER_STAR_8	/* Adjust for integer*8. */
#if defined(_WIN64) || defined(__alpha__) || defined(__sparc64__) || defined(__x86_64__) || defined(__ia64__)
typedef long longint;		/* system-dependent */
typedef unsigned long ulongint;	/* system-dependent */
#else
typedef long long longint;		/* system-dependent */
typedef unsigned long long ulongint;	/* system-dependent */
#endif
#define qbit_clear(a,b)	((a) & ~((ulongint)1 << (b)))
#define qbit_set(a,b)	((a) |  ((ulongint)1 << (b)))
#endif

#endif // F2C_INCLUDE

#endif // _AVERISERA_F2C_TYPES_H
