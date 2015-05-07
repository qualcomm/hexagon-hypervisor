# TRACE32 Extension Development Kit 
# Makefile to cross-build crt0.o, libc.a and libt32.a
# Copyright Lauterbach GmbH

GNULIB  := .
INCDIR  := ../inc
GNUDIR  := ../bin

# limit execution PATH to cross-gcc tools 
PATH    := $(GNUDIR)

RM      := $(GNUDIR)/rm
LIBCC   := $(GNUDIR)/xgcc
LIBAR   := $(GNUDIR)/ar

LIBMODS := 101 102 103 104 105 106 107 108 109 10a 10b 10c 10d 10e 10f \
           110 111 112 113 114 115 116 117 118 119 11a 11b 11c 11d 11e 11f 120 121 122 123 124 125\
           126 127 128 140 141 160 161 180 \
           201 202 203 204 205 206 207 208 209 20a 20b \
           301 302 303 304 305 306 307 311 312 313 315 316 321 322 323 331 340 341 342 \
           351 352 353 354 355 360 361 362 363 \
           400 401 402 403 40f 411 412 413 421 422 431 432 433 434 435 436 437 441 451 452 453 454 \
           501 502 511 512 513 514 515 516 517 518 521 \
           601 602 603 604 605 606 607 608 701 702 703 704 705 706 707 711 712 713 714 715 718 \
           801 802 901 902 a01 a02 a11 a12 a13 a14 a21 a22 a31 a32 b01 b02 b03 b04 \
           1000 1010 1011 1020

LIBOBJS  := $(foreach MOD, $(LIBMODS), t32e$(MOD).o)

CLIBMODS:= isalnum isalpha isascii iscntrl  isdigit  isgraph  islower isprint \
           ispunct isspace isupper isxdigit toascii  tolower  toupper         \
           memchr  memcmp  memcpy  memmove  memset   strcat   strchr  strcmp  \
           strcpy  strcspn strlen  strncat  strncmp  strncpy  strpbrk         \
           strrchr strspn  strstr  strtok   strtok_r strlwr   strupr          \
           strcasecmp strncasecmp  swab

CLIBOBJS := $(foreach MOD, $(CLIBMODS), $(MOD).o)

LIBCCOPT := -c -O -I$(INCDIR) -B$(GNUDIR) -msoft-float
LIBAROPT := -rsDv

.PHONY: default all clean 

default: clean all

all: $(GNULIB)/libc.a $(GNULIB)/libt32.a $(GNULIB)/crt0.o

clean:
	$(RM) -f $(CLIBOBJS) 
	$(RM) -f $(LIBOBJS) 
	$(RM) -f $(GNULIB)/libc.a $(GNULIB)/libt32.a $(GNULIB)/crt0.o 

# build stripped-down C library
$(GNULIB)/libc.a: $(CLIBOBJS)
	$(LIBAR) $(LIBAROPT) $@ $(CLIBOBJS)
	$(RM) -f $(CLIBOBJS)

$(CLIBOBJS): $(GNULIB)/edklibc.c $(INCDIR)/edklibc.h
	$(LIBCC) $(LIBCCOPT) -DEDK_$(basename $@) -o$@ $< 

# build TRACE32 Extension Library
$(GNULIB)/libt32.a: $(LIBOBJS)
	$(LIBAR) $(LIBAROPT) $@ $(LIBOBJS)
	$(RM) -f $(LIBOBJS)

$(LIBOBJS): $(GNULIB)/libt32.c $(INCDIR)/t32ext.h
	$(LIBCC) $(LIBCCOPT) -D$(subst t32e,T32EXT,$(basename $@)) -o$@ $< 

# build C run-time iniitalization file
$(GNULIB)/crt0.o: $(GNULIB)/crt0.S
	$(LIBCC) $(LIBCCOPT) -o$@ $<

# eof

