Q6VERSION = v73
CC      = hexagon-clang   -m$(Q6VERSION)
CXX     = hexagon-clang++ -m$(Q6VERSION)
LIBDL    = $(shell $(CC) -mv73 -G0 -print-file-name=libdl.a)
LIBC_EH  = $(shell $(CC) -mv73 -G0 -print-file-name=libc_eh.a)
H2DIR   = ..

COBJS   = libdltest.o fstat.o sys_mmap.o
CXXOBJS = cpptest.o

run: libdltest french.so cppso.so
	PKW_arch=v73_stable archsim --quiet $(H2DIR)/artifacts/v73/opt/install/bin/booter libdltest

french.so: french.c
	$(CC) -fPIC -Wl,-Bsymbolic -shared -nostartfiles -o french.so french.c

cppso.so: cppso.cpp
	$(CXX) -fPIC -Wl,-Bsymbolic -shared -o cppso.so cppso.cpp

$(COBJS): %.o: %.c
	$(CC) -c -o $@ $<

$(CXXOBJS): %.o: %.cpp
	$(CXX) -c -o $@ $<

libdltest: $(COBJS) $(CXXOBJS)
	$(CXX) -moslib=h2 -Wl,-Bsymbolic -Wl,--export-dynamic -Wl,--force-dynamic \
	    -Wl,--dynamic-linker= -Wl,--allow-multiple-definition \
	    -L$(H2DIR)/artifacts/v73/opt/install/lib \
	    -o libdltest $(COBJS) $(CXXOBJS) $(LIBDL) \
	    -Wl,--whole-archive $(LIBC_EH) -Wl,--no-whole-archive

clean:
	rm -f libdltest french.so cppso.so $(COBJS) $(CXXOBJS) pmu_stats.txt

.PHONY: run clean
