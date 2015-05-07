/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** Stripped-down C library for TRACE32 Extension Programmer.
   "libc.a" functions and xgcc-builtins.
   see http://www.delorie.com/djgpp/doc/libc/
   see http://gcc.gnu.org/onlinedocs/gcc-2.95.3/gcc_4.html
**/

/* helper definition for memchr, memcmp, memcpy, memmove, memset */
typedef unsigned char byte;

/* stddef.h -- gcc-specific implementation */
typedef __typeof__(sizeof(int)) size_t;

/* ctype.h */
#ifdef EDK_isalnum
int      isalnum(int ch)
{
    return ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) || ((ch >= '0') && (ch <= '9'));
    /* or: (isalpha(ch) || isdigit(ch)) */
}
#endif

#ifdef EDK_isalpha
int      isalpha(int ch)
{
    return ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z'));
}
#endif

#ifdef EDK_isascii
int      isascii(int ch)
{
    return (ch < 128);
}
#endif

#ifdef EDK_iscntrl
int      iscntrl(int ch)
{
    return (ch < 32) || (ch == 127);
}
#endif

#ifdef EDK_isdigit
int      isdigit(int ch)
{
    return (ch >= '0') && (ch <= '9');
}
#endif

#ifdef EDK_isgraph
int      isgraph(int ch)
{
    return (ch > 32) && (ch < 127);
}
#endif

#ifdef EDK_islower
int      islower(int ch)
{
    return (ch >= 'a') && (ch <= 'z');
}
#endif

#ifdef EDK_isprint
int      isprint(int ch)
{
    return (ch >= 32) && (ch < 127);
}
#endif

#ifdef EDK_ispunct
int      ispunct(int ch)
{
    return ((ch > ' ') && (ch < '0')) || ((ch > '9') && (ch < 'A'))
        || ((ch > 'Z') && (ch < 'a')) || ((ch > 'Z') && (ch < 127));
    /* or: isascii(ch) && !isalnum(ch) && !iscntrl(ch) */
}
#endif

#ifdef EDK_isspace
int      isspace(int ch)
{
    return (ch == 32) || ((ch >= 9) && (ch <= 13));
}
#endif

#ifdef EDK_isupper
int      isupper(int ch)
{
    return (ch >= 'A') && (ch <= 'Z');
}
#endif

#ifdef EDK_isxdigit
int      isxdigit(int ch)
{
    return ((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'f')) || ((ch >= 'A') && (ch <= 'F'));
}
#endif

#ifdef EDK_toascii
int      toascii(int ch)
{
    return (ch & 0x7F);
}
#endif

#ifdef EDK_tolower
int      tolower(int ch)
{
    if ((ch >= 'A') && (ch <= 'Z'))
    	ch = ch - 'A' + 'a';
    return ch;
}
#endif

#ifdef EDK_toupper
int      toupper(int ch)
{
    if ((ch >= 'a') && (ch <= 'z'))
    	ch = ch - 'a' + 'A';
    return ch;
}
#endif

/* string.h */
#ifdef EDK_memchr
void    *memchr(const void *str, int ch, size_t len)
{
    const byte *p = str;
    for(;len--;p++) if (*p == (byte)ch) return ((void *)p);
    return 0;
}
#endif

#ifdef EDK_memcmp
int      memcmp(const void *str1, const void *str2, size_t len)  /* builtin */
{
    const byte *p = str1;
    const byte *q = str2;
    int r = 0;
    while (len--) {
    	if ((r = (*p - *q)) != 0)
    	    break;
    	p++;
    	q++;
    }
    return r;

}
#endif

#ifdef EDK_memcpy
void    *memcpy(void *dst, const void *src, size_t len) /* builtin */
{
    byte *d = dst;
    const byte *s = src;
    while (len--) *d++ = *s++;
    return dst;
}
#endif

#ifdef EDK_memmove
void    *memmove(void *dst, const void *src, size_t len)
{
    byte *d = dst;
    const byte *s = src;
    if (d > s) {
    	s += len;
    	d += len;
	while (len--) *--d = *--s;
    } else {
	while (len--) *d++ = *s++;
    }
    return dst;
}
#endif

#ifdef EDK_memset
void    *memset(void *dst, int val, size_t len)
{
    byte *p, v;
    for (p = dst, v = (byte)val; len--; p++) *p = v;
    return dst;
}
#endif

#ifdef EDK_strcat
char    *strcat(char *dst, const char *src)
{
    char *d = dst;
    const char *s = src;
    while (*d) d++;
    while ((*d++ = *s++)) /*noop*/;
    return dst;
}
#endif

#ifdef EDK_strchr
char    *strchr(const char *str, int ch)
{
    for (; *str; str++)
    	if (*str == (char)ch)
	    return (char *)str;
    return 0;
}
#endif

#ifdef EDK_strcmp
int      strcmp(const char *str1, const char *str2) /* builtin */
{
    while(*str1 && (*str1 == *str2)) { str1++; str2++; }
    return (*str1 - *str2);
}
#endif

#ifdef EDK_strcpy
char    *strcpy(char *dst, const char *src) /* builtin */
{
    char *s = dst;
    while(*s++ = *src++) /*noop*/;
    return dst;
}
#endif

#ifdef EDK_strspn
size_t   strspn(const char *str, const char *accept)
{
    size_t n = 0;
    while (*str) {
	const char *a;
	for (a = accept; *a && (*str != *a); a++) /*noop*/;
	if (!*a) break;
    	str++;
        n++;
    }
    return n;
}
#endif

#ifdef EDK_strcspn
size_t   strcspn(const char *str, const char *reject)
{
    const char *m, *s = str;
    while (*s) {
    	for (m = reject; *m && (*s != *m); m++) /*noop*/;
    	if (*m)
    	    break;
    	s++;
    }
    return s - str;
}
#endif

#ifdef EDK_strlen
size_t   strlen(const char *str) /* builtin */
{
    size_t n = 0;
    while (*str++) n++;
    return n;
}
#endif

#ifdef EDK_strncat
char    *strncat(char *dst, const char *src, size_t len)
{
    char *d = dst;
    const char *s = src;
    while (*d) d++;
    while (len--)
    	if (!(*d++ = *s++))
    		return dst;
    *d = '\0';
    return dst;
}
#endif

#ifdef EDK_strncmp
int      strncmp(const char *str1, const char *str2, size_t len)
{
    while (len--) {
		unsigned char c = (unsigned char) *str1;
		int d = c - (unsigned char) *str2;
		if (d || !c) return d;
		str1++;
		str2++;
	}
    return 0;
}
#endif

#ifdef EDK_strncpy
char    *strncpy(char *dst, const char *src, size_t len)
{
    char *d = dst;
    while (len && (*d++ = *src++)) len--;
    while (len) {*d++ = '\0'; len--;} /* fill up with zeroes */
    return dst;
}
#endif

#ifdef EDK_strpbrk
char    *strpbrk(const char *str, const char *accept)
{
    char c;
    while (c = *str++) {
	const char *a;
	for (a = accept; *a; a++)
	    if (c == *a)
	    	return (char *)a;
	str++;
    }
    return 0;
}
#endif

#ifdef EDK_strrchr
char    *strrchr(const char *str, int ch)
{
    char *s, *f = 0;
    for (s = (char *)str; *s; s++)
    	if (*s == (char)ch)
	    f = s;
    return f;
}
#endif

#ifdef EDK_strstr
char    *strstr(const char *haystack, const char *needle)
{
    const char *haytop = haystack;
    const char *neepos = needle;

    while (*haystack) {
	if (!*neepos)
	    return (char *)haytop;	/* we found it (also, if needle is empty) */

    	if (*haystack == *neepos) {
	    haystack++;
	    neepos++;
    	} else {
    	    neepos   = needle;   /* reset to start of needle */
    	    haystack = ++haytop; /* search continues at next char */
    	}
    }

    if (!*neepos)
	return (char *)haytop;	/* needle was right-aligned in haystack */

    return 0;			/* not found */
}
#endif

#ifdef EDK_strtok_r
size_t   strspn(const char *str, const char *accept);  /* prototype */
size_t   strcspn(const char *str, const char *reject); /* prototype */
char    *strtok_r(char *src, const char *delim, char **saveptr)
{
    char *pos = 0;
    if (!src) src = *saveptr;
    src += strspn(src,delim);      	/* skip leading delimiters */
    if (*src) {
    	pos = (char *)src;
    	src += strcspn(src,delim); 	/* skip until next delimiter */
    	if (*src) { *src = 0; src++; }	/* null-terminate token */
    }
    *saveptr = src;
    return pos;

}
#endif

#ifdef EDK_strtok
char    *strtok_r(char *src, const char *delim, char **saveptr); /* prototype */
char    *strtok(char *src, const char *delim)
{
    static char *saveptr;
    return strtok_r(src, delim, &saveptr);
}
#endif

/* string.h (non-portable) */

#ifdef EDK_strlwr
char    *strlwr(char *str)
{
    char c, *d = str;
    while (c = *d) {
	 if ((c >= 'A') && (c <= 'Z'))
	     *d = (c - 'A' + 'a');
	 d++;
    }
    return str;
}
#endif

#ifdef EDK_strupr
char    *strupr(char *str)
{
    char c, *d = str;
    while (c = *d) {
	 if ((c >= 'a') && (c <= 'z'))
	     *d = (c - 'a' + 'A');
	 d++;
    }
    return str;
}
#endif

/* strings.h */
#ifdef EDK_strcasecmp
int      strcasecmp(const char *str1, const char *str2)
{
    unsigned char s, t;
    do {
    	s = (unsigned char) *str1++;
    	t = (unsigned char) *str2++;
    	if (s != t) {
	    if ((s >= 'A') && (s <= 'Z')) s = s - 'A' + 'a';
	    if ((t >= 'A') && (t <= 'Z')) t = t - 'A' + 'a';
	    if (s != t) return (s - t);
	}
    } while (s);
    return 0;
}
#endif

#ifdef EDK_strncasecmp
int      strncasecmp(const char *str1, const char *str2, size_t len)
{
    unsigned char s, t;
    do {
    	if (!len--) break;
    	s = (unsigned char) *str1++;
    	t = (unsigned char) *str2++;
    	if (s != t) {
	    if ((s >= 'A') && (s <= 'Z')) s = s - 'A' + 'a';
	    if ((t >= 'A') && (t <= 'Z')) t = t - 'A' + 'a';
	    if (s != t) return (s - t);
	}
    } while (s);
    return 0;
}
#endif

/* stdlib.h */

#ifdef EDK_swab
void swab(const void *src, void *dst, int len)
{
    const char *s = src;
    char *d = dst;
    int i = (len & ~1);
    while (i > 0) {
    	--i;
    	d[i] = s[i-1];
    	--i;
    	d[i] = s[i+1];
    }
}
#endif

/*   With (x)gcc:  abs, labs, ffs, memcmp, memcpy, strcmp, strcpy, strlen
     - are usually builtins, but e.g. memcmp, memcpy, strcmp, strcpy, strlen
     - can be called externally if -fno-builtins flag is set during compile
     - can be called externally if memory block or string size is too large (!)

     NOT IMPLEMENTED:
     alloca             - allocation from local stack (normally done by compiler)
     fabsf, fabs, fabsl - absolute value for floating point variables
     sqrtf, sqrt, sqrtl - square root functions
     sinf, sin, sinl    - trigonometric functions
     cosf, cos, cosl    - trigonometric functions

     NOT COMPILED:
     ffs, abs, fabs     - demonstration / in case your cross-gcc does not have them
*/

/* strings.h */
#ifdef EDK_ffs
int ffs(int __i) /* GCC built-in - not compiled, demonstration only */
{ /* return index of lowest set bit (0 for none, 1..sizeof(int) otherwise) */
    int p = 1;
    if (!__i) return 0;
    while (!(__i & 1)) {__i >>= 1; p++;}
    return p;
}
#endif

/* stdlib.h */
#ifdef EDK_abs
int      abs(int val) /* GCC built-in - not compiled, demonstration only */
{
    return (val < 0) ? (-val) : (val);
}
#endif

#ifdef EDK_labs
long     labs(long lval)  /* GCC built-in - not compiled, demonstration only */
{
    return (lval < 0) ? (-lval) : (lval);
}
#endif

/*EOF**/
