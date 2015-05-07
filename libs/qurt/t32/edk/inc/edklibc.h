/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** Stripped-down C library for TRACE32 Extension Programmer. 
   "libc.a" functions and xgcc-builtins.
   see http://www.delorie.com/djgpp/doc/libc/
   see http://gcc.gnu.org/onlinedocs/gcc-2.95.3/gcc_4.html
**/

/* stddef.h -- gcc-specific implementation */
typedef __typeof__(sizeof(int)) size_t;

/* ctype.h */
int      isalnum(int ch);
int      isalpha(int ch);
int      isascii(int ch);
int      iscntrl(int ch);
int      isdigit(int ch);
int      isgraph(int ch);
int      islower(int ch);
int      isprint(int ch);
int      ispunct(int ch);
int      isspace(int ch);
int      isupper(int ch);
int      isxdigit(int ch);

int      toascii(int ch);
int      tolower(int ch);
int      toupper(int ch);

/* stdlib.h */
int      abs(int val);    /* BUILTIN */
long     labs(long lval); /* BUILTIN */
void     swab(const void *src, void *dst, int len);

/* string.h */
int      ffs(int __i);    /* BUILTIN, find first set bit - 1..sizeof(int), 0 if none */

void    *memchr(const void *str, int ch, size_t len);
int      memcmp(const void *str1, const void *str2, size_t len);
void    *memcpy(void *dst, const void *src, size_t len);
void    *memmove(void *dst, const void *src, size_t len);
void    *memset(void *dst, int val, size_t len);

char    *strcat(char *dst, const char *src);
char    *strchr(const char *str, int ch);
int      strcmp(const char *str1, const char *str2);
char    *strcpy(char *dst, const char *src);
size_t   strcspn(const char *str, const char *reject);
size_t   strlen(const char *str);
char    *strncat(char *dst, const char *src, size_t len);
int      strncmp(const char *str1, const char *str2, size_t len);
char    *strncpy(char *dst, const char *src, size_t len);
char    *strpbrk(const char *str, const char *accept);
char    *strrchr(const char *str, int ch);
size_t   strspn(const char *str, const char *accept);
char    *strstr(const char *haystack, const char *needle);
char    *strtok(char *src, const char *delim);
char    *strtok_r(char *src, const char *delim, char **saveptr);

char    *strlwr(char *str);  /* non-portable, convert string to lower case */
char    *strupr(char *str);  /* non-portable, convert string to upper case */
#define stricmp  strcasecmp  /* non-portable, case-insensitive compare */  
#define strnicmp strncasecmp /* non-portable */

/* strings.h */
int      strcasecmp(const char *str1, const char *str2);
int      strncasecmp(const char *str1, const char *str2, size_t len);
/* identical functions -- not implemented twice
  - for index    please use  strchr
  - for rindex   please use  strrchr
  (cannot '#define': 'index' might be a variable name)
*/

#ifndef _SUPPRESS_BUILTIN_
/* BUILTIN - These functions are NOT implemened in edklibc.c
   If not suppressed, xgcc uses built-in statements for them.
*/
/* stdlib.h */
void    *alloca(size_t __n);
/* math.h */
double   fabs(double __d);
double   sqrt(double __d);
double   sin(double __d);
double   cos(double __d);
float    fabsf(float __f);
float    sqrtf(float __f);
float    sinf(float __f);
float    cosf(float __f);
long double fabsl(long double __d);
long double sqrtl(long double __d);
long double sinl(long double __d);
long double cosl(long double __d);
#endif /*_SUPPRESS_BUILTIN_*/

