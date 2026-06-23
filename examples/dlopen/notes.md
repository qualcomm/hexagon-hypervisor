# dltest — Notes for using as an example

NOTE: if you get an unsupported sys_mmap error, try removing the entire artifacts
directory from H2 and rebuilding.

This directory demonstrates `dlopen`/`dlsym` working under h2/booter on Hexagon v73.
It loads both a C shared library (`french.so`) and a C++ shared library (`cppso.so`)
from a statically-linked main binary, with full constructor/destructor support.

## Required: booter must include the elf_get_specials fix (commit 13cecf0f)

The main binary is built with `--force-dynamic`, which makes it a dynamic ELF binary
with two string tables: `.dynstr` (for `.dynsym`) and `.strtab` (for `.symtab`).

Older booter versions scan for the first `SHT_STRTAB` section by type to decode symbol
names in `.symtab`.  In a dynamic binary `.dynstr` comes first in section order, so the
old code pairs `.symtab` with `.dynstr` — causing all `strcmp` lookups for boot symbols
to fail silently.  The result: `__boot_cmdline__`, `end`, `__boot_net_phys_offset__`,
and `__sys_write_mode__` are never found, the heap and cmdline are not set up, and the
binary crashes or produces no output.

The fix (commit `13cecf0f`, "elf: fix elf_get_specials strtab selection for dynamic
binaries") uses `sh_link` (called `sh_line` in h2_elf.h) on the `.symtab` section
header to find its correct paired string table by index rather than scanning by type.

**Any binary built with `--force-dynamic` requires a booter built from commit
`13cecf0f` or later.**

## What each file does

| File | Purpose |
|------|---------|
| `libdltest.c` | Main program: calls `dlinit`, `dlopen`, `dlsym`, calls functions |
| `cpptest.cpp` | C++ global object in the main binary — proves main-binary ctors work |
| `french.c` | Simple C shared lib: `faux()` calls `printf` and returns a string |
| `cppso.cpp` | C++ shared lib: static global (tests .so ctors), exports `cppfunc()` |
| `fstat.c` | Fake `fstat` required by the rtld (angel doesn't implement SWI 0x101) |
| `sys_mmap.c` | `mmap`/`munmap`/`mprotect` via angel — required by the rtld to load .so files |
| `Makefile` | Build and run rules |
| `example/` | Standalone (non-h2) reference using `crt0_standalone + libstandalone` |

## Critical: link with hexagon-clang++, not hexagon-clang

The main binary must be linked with `hexagon-clang++`.  Without it, `--force-dynamic`
adds `.ctors`/`.dtors`/`_init` sections, and the h2 startup calls `_init` before
`main()`.  One of the constructors (`__register_frame_info_base` from libgcc) crashes
silently — the VM runs ~48k instructions and produces no output.

`hexagon-clang++` links the full C++ runtime which provides working implementations of
those frame-registration functions.

**If your app is C-only** and you need `--force-dynamic`, you must provide a no-op
`_init`/`_fini` to suppress the crashing constructor:

```c
/* noinit.c — include this in C-only builds that use --force-dynamic */
void _init(void) {}
void _fini(void) {}
```

Link with `-Wl,--allow-multiple-definition` so these override the ones from libgcc.
Note: this suppresses ALL main-binary constructors, so C++ global objects won't work.

## Why --force-dynamic and -E are required

The Hexagon rtld resolves undefined symbols in `.so` files from loaded objects.
For `french.so` to call `printf`, `printf` must be in some loaded object's symbol table.

`-Wl,--force-dynamic -Wl,--export-dynamic` forces the linker to create a `.dynsym`
section that exports all global symbols from the main binary.  Without this, a
static binary has no `.dynsym` and `-rdynamic`/`--export-dynamic` are silently ignored
by Hexagon LLD.

`dlinit` then tells the rtld these exported symbols correspond to named "builtin"
libraries (e.g. "libc.so"), so when a loaded `.so` has `NEEDED: libc.so`, the rtld
satisfies it from the main binary rather than loading a file.

## The dlinit builtins list

```c
const char *builtin[] = {"libgcc.so", "libc.so", "libstdc++.so",
                         "libc++.so.1", "libc++abi.so.1"};
dlinit(5, (char **)builtin);
```

Each name is a library that your `.so` files may list in their `NEEDED` section.
The rtld satisfies these from the main binary's exported symbol table rather than
opening files.  Key points:

- `libc++.so.1` and `libc++abi.so.1` are needed for C++ `.so` files built with
  `hexagon-clang++` (LLVM's libc++ is the default C++ runtime).
- If your `.so` is C-only (built with `-nostartfiles`), you can omit the libc++ entries.
- `libgcc.so` is needed for low-level runtime helpers.

## Why libc_eh.a is linked with --whole-archive

`libc++abi` (the C++ ABI library) calls `_Get_eh_data` from the Hexagon exception
handling library (`libc_eh.a`).  The `-moslib=h2` system library does not include this.

Since nothing in the main binary's own code directly calls `_Get_eh_data`, the linker
would normally drop it.  `--whole-archive` forces all symbols from `libc_eh.a` to be
included and exported, so the rtld can resolve them when loading the C++ `.so`.

If your app is C-only, you can skip `libc_eh.a`.

## Why fstat is faked

The Hexagon angel semihosting interface does not implement `fstat` (SWI 0x101).
The rtld calls `fstat` to get `st_dev`/`st_ino` before loading each library — it uses
these to detect duplicate loads (same dev+ino = already loaded, return cached handle).

The fake `fstat` uses an incrementing counter for `st_ino` to ensure each file always
appears as a distinct inode.  Using `st_ino = fd` (a natural fallback) breaks things:
two `.so` files opened on the same fd number get the same inode, and the rtld returns
the first library's handle for both, causing the wrong symbols to be found.

## Why sys_mmap.c is needed

The rtld calls `mmap` to map loaded `.so` files into memory.  The h2 environment does
not provide `mmap` by default.  `sys_mmap.c` implements it using the angel `sys_read`
and `sys_seek` calls: `MAP_FIXED` maps read the file into a provided address, and
anonymous/non-fixed maps use `memalign` to allocate heap memory.

`munmap` is a no-op (leaking is acceptable for dltest; dlclose is not supported).

## dlopen path requirements

The Hexagon rtld requires an absolute path for `dlopen` unless the library is listed
as a dlinit builtin:

- `dlopen("french.so", ...)` — works if `"french.so"` is a dlinit builtin (symbols
  resolved from main binary, no file opened).
- `dlopen("./french.so", ...)` — FAILS: "abs pathname required".
- `dlopen("/abs/path/french.so", ...)` — works but requires the rtld to load the file.
  Note: the file's own `_init` will run, which may crash in the h2 environment.

For a file-loaded `.so` to work (rather than as a builtin), its `_init` must be safe
in h2, and all its NEEDED libraries must be pre-loaded or also be builtins.

## Building a C++ shared library

```makefile
CXX = hexagon-clang++ -mv73
cppso.so: cppso.cpp
    $(CXX) -fPIC -Wl,-Bsymbolic -shared -o cppso.so cppso.cpp
```

Do **not** use `-nostartfiles` for C++ `.so` files.  Without it, `hexagon-clang++`
sets up a proper `_init` (via `DT_INIT` in `.dynamic`) that iterates `.ctors` and
calls C++ global constructors.  With `-nostartfiles` this infrastructure is absent
and constructors silently do not run.

Note: `crtbeginS.o`/`crtendS.o` exist in the toolchain's `pic/` directory but use
non-PIC relocations and cannot be linked into a shared library — let the driver handle
it automatically.

## libdl.a must match the target arch

Use `libdl.a` from the same arch as your target binary.  A v68 `libdl.a` linked into
a v73 binary causes `dlinit` to hang (jump-to-self infinite loop).

```makefile
LIBDL = $(shell hexagon-clang -mv73 -G0 -print-file-name=libdl.a)
```

## Adapting for your own app

1. Copy `fstat.c` and `sys_mmap.c` into your project (or promote to a shared library).
2. Add them to your link line alongside `libdl.a`.
3. Link with `hexagon-clang++` (even if your app is pure C — it's the safe choice).
4. Add `-Wl,--force-dynamic -Wl,--export-dynamic -Wl,--dynamic-linker=` to your
   link flags.
5. Add `-Wl,--allow-multiple-definition` to handle any symbol conflicts.
6. Add `-Wl,--whole-archive libc_eh.a -Wl,--no-whole-archive` for C++ `.so` support.
7. Call `dlinit` at the start of `main()` with the builtins list appropriate for your
   `.so` files.
8. Build `.so` files with `hexagon-clang++` (no `-nostartfiles`) for C++ ctors, or
   `hexagon-clang -nostartfiles` for C-only libraries.
