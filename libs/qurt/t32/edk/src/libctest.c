/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** TRACE32 Extension Development Kit,
 *  libc test extension module.
 *  Copyright Lauterbach GmbH
 */
#include "edklibc.h"
#include "t32ext.h"

#define _bug_workaround_mandatory_data
#define _bug_workaround_eight_parameter_limit


void test_ctype_h(void)
{
    char *ts = "T32: \tSüD$d"; //"TRACE32_;$ Würfel\taeäoeöueü~§Test";
    int c, i;

    T32Printf("ctype.h-test: String is '%s'\n", ts);
    for (i = 0; (c = ts[i]) != 0; i++) {
#ifdef _bug_workaround_eight_parameter_limit
	T32Printf("    '%c' - is(alnum:%c alpha:%c ascii:%c cntrl:%c digit:%c graph:%c ", c,
	          isalnum(c)?'Y':'N', isalpha(c)?'Y':'N', isascii(c)?'Y':'N',
	          iscntrl(c)?'Y':'N', isdigit(c)?'Y':'N', isgraph(c)?'Y':'N');
	T32Printf("lower:%c print:%c punct:%c space:%c upper:%c xdigit:%c)\n",
	          islower(c)?'Y':'N', isprint(c)?'Y':'N', ispunct(c)?'Y':'N',
	          isspace(c)?'Y':'N', isupper(c)?'Y':'N', isxdigit(c)?'Y':'N');
	T32Printf("         to(ascii:%c lower:%c upper:%c)\n",
	          (char)toascii(c), (char)tolower(c), (char)toupper(c));
#else
    // Life's little surprises: T32Printf can only process max. 8 parameters. Urgh!
	T32Printf("    '%c' - is(alnum:%c alpha:%c ascii:%c cntrl:%c "
	          "digit:%c graph:%c lower:%c print:%c "
	          "punct:%c space:%c upper:%c xdigit:%c)\n"
	          "         to(ascii:%c lower:%c upper:%c)\n", c,
	          isalnum(c)?'Y':'N', isalpha(c)?'Y':'N', isascii(c)?'Y':'N', iscntrl(c)?'Y':'N',
	          isdigit(c)?'Y':'N', isgraph(c)?'Y':'N', (islower(c))?'Y':'N', (isprint(c))?'Y':'N',
	          ispunct(c)?'Y':'N', isspace(c)?'Y':'N', isupper(c)?'Y':'N', isxdigit(c)?'Y':'N',
	          (char)toascii(c), (char)tolower(c), (char)toupper(c)
	          );
#endif // _bug_workaround_eight_parameter_limit
    } //for
    T32Printf("    info: isalnum() adds ctype_.o, 0x104 bytes in .rodata\n");
#ifdef _bug_workaround_mandatory_data
    {
    // Life's little surprises: extensions must have a non-empty DATA segment
    static char buffer[200];
#ifdef __include_libc_memcpy
    // SIDE_EFFECT: this also includes libc.a-memcpy (64 bytes is too large for "builtin" version)
    strcpy(buffer, "ctype.h-test ----------------------------------------------END.");
    T32Printf("%s\n", buffer);
#else
    strcpy(buffer, "ctype.h");
    T32Printf("%s-test ----------------------------------------------END.\n", buffer);
#endif
    }
#else
    T32Printf("ctype.h-test ----------------------------------------------END.\n");
#endif // _bug_workaround_mandatory_data
}


void test_string_h(void)
{
    char tb[32];

    T32Printf("\nstring.h-test:\n  Builtins:\n");

    T32Printf("    memcmp('test','t_st',5) == %d   - ", memcmp("test","t_st",5));
    T32Printf("memcmp('test','test',5) == %d\n",  memcmp("test","test",5));

    T32Printf("    strcmp('test','tast')   == %d   - ", strcmp("test","tast"));
    T32Printf("strcmp('test','test')   == %d   - ",  strcmp("test","test"));
    T32Printf("strcmp('test','txst')   == %d\n",  strcmp("test","txst"));


    T32Printf("    memcpy(tb,'test',5) == '%s'  - ", memcpy(tb, "test",5));
    T32Printf("strcpy(tb,'test ok') == '%s'\n", strcpy(tb, "test ok"));

    T32Printf("    memchr('test ok','s',7) -> '%c' - ", *(char *)memchr("test ok",'s',7));
    T32Printf("memchr('test ok','o',7) -> '%c'\n",  *(char *)memchr("test ok",'o',7));

    T32Printf("    strchr('test ok','s')   -> '%c' - ", *strchr("test ok",'s'));
    T32Printf("strchr('test ok','k')   -> '%c'\n",  *strchr("test ok",'k'));

    memcpy(tb,"test ok",8);
    T32Printf("    (1) tb == '%s' ", tb);
    T32Printf("(2) strcat(tb,'yoku') == '%s'", strcat(tb,"yoku"));
    T32Printf("(3) tb == '%s'\n", tb);

    T32Printf("    strspn('test ok','et')  == %d   - ", strspn("test ok","et"));
    T32Printf("strcspn('test ok',' o') == %d\n",  strcspn("test ok"," o"));

    T32Printf("  Library:\n");

    T32Printf("    strlen('test')          == %d   - ", strlen("test"));
    T32Printf("strlen('test string')   == %d\n",  strlen("test string"));

    memcpy(tb,"test ok",8);
    T32Printf("    (1) tb == '%s' ", tb);
    T32Printf("(2) memset( tb+2,64,3) == '%s'     ", memset(tb+2,64,3));
    T32Printf("(3) tb == '%s'\n", tb);

    memcpy(tb,"test ok",8);
    T32Printf("    (1) tb == '%s' ", tb);
    T32Printf("(2) memmove(tb,tb+2,5) == '%s'   ", memmove(tb,tb+2,5));
    T32Printf("(3) tb == '%s'\n", tb);

    memcpy(tb,"test ok",8);
    T32Printf("    (1) tb == '%s' ", tb);
    T32Printf("(2) strncmp(tb,'test',4) == %d", strncmp(tb,"test",4));
    T32Printf("         (3) strncmp(tb,'test',5) == %d\n", strncmp(tb,"test",5));
    T32Printf("    (4) strncmp(tb,'tast',4) == %d    ", strncmp(tb,"tast",4));
    T32Printf("(5) strncmp(tb,'txst',4) == %d\n", strncmp(tb,"txst",4));


    /*
    char    *strncat(char *__s1, const char *__s2, size_t __n);
    int      strncmp(const char *__s1, const char *__s2, size_t __n);
    char    *strncpy(char *__s1, const char *__s2, size_t __n);
    char    *strpbrk(const char *__s1, const char *__s2);
    char    *strrchr(const char *__s1, int __c);
    char    *strstr(const char *__s1, const char *__s2);
    char    *strtok(char *__s1, const char *__s2);
    char    *strtok_r(char *__s1, const char *__s2, char **__as3);
    */
    T32Printf("string.h-test ---------------------------------------------END.\n");

    T32Printf("Display Tests\n");
    T32Printf("(1) Display String\n");
    T32DisplayString("abcd1234567! §$%&/()");
    T32Printf("[post]\n");
    T32Printf("(2) Display String Fix 16\n");
    T32DisplayStringFix("abcd1234567! §$%&/()", 16);
    T32Printf("[post]\n");
    T32Printf("(3) Display String Fix 32\n");
    T32DisplayStringFix("abcd1234567! §$%&/()", 32);
    T32Printf("[post]\n");
    T32Printf("(4) Display 0x12345678 Dec3 - Dec5 - Hex2 - Hex4 - Hex8 - (-1) Dec3 / Dec5\n");
    T32DisplayDec3(0x12345678);
    T32DisplayString(" - ");
    T32DisplayDec5(0x12345678);
    T32DisplayString(" - ");
    T32DisplayHex2(0x12345678);
    T32DisplayString(" - ");
    T32DisplayHex4(0x12345678);
    T32DisplayString(" - ");
    T32DisplayHex8(0x12345678);
    T32DisplayString(" - ");
    T32DisplayDec3(-1);
    T32DisplayString(" / ");
    T32DisplayDec5(-1);
    T32DisplayLf(1);
    T32Printf("(5) Display 99999 Dec3 - Dec5 - Hex2 - Hex4 - Hex8 - (-2) Dec3 / Dec5\n");
    T32DisplayDec3(99999);
    T32DisplayString(" - ");
    T32DisplayDec5(99999);
    T32DisplayString(" - ");
    T32DisplayHex2(99999);
    T32DisplayString(" - ");
    T32DisplayHex4(99999);
    T32DisplayString(" - ");
    T32DisplayHex8(99999);
    T32DisplayString(" - ");
    T32DisplayDec3(-2);
    T32DisplayString(" / ");
    T32DisplayDec5(-2);
    T32DisplayLf(1);
    T32Printf("(6) Display 9 Dec3 - Dec5 - Hex2 - Hex4 - Hex8 - (-3) Dec3 / Dec5\n");
    T32DisplayDec3(9);
    T32DisplayString(" - ");
    T32DisplayDec5(9);
    T32DisplayString(" - ");
    T32DisplayHex2(9);
    T32DisplayString(" - ");
    T32DisplayHex4(9);
    T32DisplayString(" - ");
    T32DisplayHex8(9);
    T32DisplayString(" - ");
    T32DisplayDec3(-3);
    T32DisplayString(" / ");
    T32DisplayDec5(-3);
    T32DisplayLf(1);
    T32Printf("t32ext.h-test ---------------------------------------------END.\n");

}


void TestDisplay(void)
{
    test_ctype_h();
    test_string_h();
}


/* Initialization of test */
T32CmdDisplayDef *TestInit(void)
{
    static const char header[] = "--- EDK libc test ---";
    static T32CmdDisplayDef display = { WINDOW, 0, 120, header, TestDisplay };
    return &display;
}

/* Parser: returns initialization routine */
T32CmdInitRtn TestParser(void)
{
    return TestInit;
}

/* Register this EXTension with TRACE32. */
void main(void)
{
    static T32CmdDef test = { "LIBTEST", "LT", "LibTest", "__EDKLT__LT", TestParser};
    T32DefineCommand(&test);
}

/*eof*/
