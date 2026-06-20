#ifndef STDDEF_H
#define STDDEF_H

/* limit of size_t */
#ifdef __SIZE_MAX__
#define	SIZE_MAX	__SIZE_MAX__			/* size_t	  */
#else
#define	SIZE_MAX	0xffffffffUL			/* size_t	  */
#endif

#ifndef size_t
typedef unsigned int                size_t;
#define size_t                      size_t
#endif

#ifndef ssize_t
typedef int                         ssize_t;
#define ssize_t                     ssize_t
#endif

#undef                              NULL
#ifndef __cplusplus
#define NULL                        ((void *)0)
#else   /* C++ */
#define NULL                        0
#endif  /* C++ */

#endif

