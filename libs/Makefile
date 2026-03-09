# Smaller kludge here

ifeq ($(ARCHV),)
export ARCHV := 3
endif

SHELL=/bin/bash

ifeq (,$(IMPL))
IMPL=ref
endif

H2_DIR=h2/
H2_COMPAT_DIR=h2_compat/
POSIX_DIR=posix/
ifeq (ref,$(IMPL))
H2_LIB=$(H2_DIR)build/ref/libh2.a
H2_COMPAT_LIB=$(H2_COMPAT_DIR)build/ref/libh2_compat.a
POSIX_LIB=$(POSIX_DIR)build/ref/libposix.a
else
IMPL=opt
H2_LIB=$(H2_DIR)build/opt_v$(ARCHV)/libh2.a
H2_COMPAT_LIB=$(H2_COMPAT_DIR)build/opt_v$(ARCHV)/libh2_compat.a
POSIX_LIB=$(POSIX_DIR)build/opt_v$(ARCHV)/libposix.a
endif

#MOST_LIBS= qube/libqube.a syscall/angel/libangel.a qurt/libqurt.a 
MOST_LIBS= syscall/angel/libangel.a qurt/libqurt.a

# Picolibc support: if PICOLIBC=1, build syscall wrapper and locks
ifeq ($(PICOLIBC),1)
export PICOLIBC
SYS_WRAPPER_LIB = syscall/wrapper/libsyscall_wrapper.a
LOCKS_LIB = locks/liblocks.a
else
SYS_WRAPPER_LIB =
LOCKS_LIB =
endif

ALL_LIBS= $(H2_LIB) $(POSIX_LIB) $(H2_COMPAT_LIB) $(MOST_LIBS)
LIB_DIRS= $(dir $(MOST_LIBS) $(H2_DIR) $(POSIX_DIR) $(H2_COMPAT_DIR) $(SYS_WRAPPER_LIB) $(LOCKS_LIB))

MAKE_LIB=${MAKE} -C $(dir $@); cp $@ tmp/


tmp:
	mkdir $@

$(H2_LIB): tmp
	+${MAKE} -C $(H2_DIR) $(IMPL); cp $@ tmp/

$(POSIX_LIB): tmp $(H2_LIB)
	+${MAKE} -C posix $(IMPL); cp $@ tmp/;

$(H2_COMPAT_LIB): tmp $(H2_LIB) $(POSIX_LIB)
	+${MAKE} -C $(H2_COMPAT_DIR) $(IMPL); cp $@ tmp/

qurt/libqurt.a: tmp $(H2_LIB) $(H2_COMPAT_LIB) $(POSIX_LIB)
	+${MAKE_LIB};

syscall/angel/libangel.a: tmp $(H2_LIB) $(H2_COMPAT_LIB) $(POSIX_LIB)
	+${MAKE_LIB};

ifeq ($(PICOLIBC),1)
$(SYS_WRAPPER_LIB):
	+${MAKE_LIB};

$(LOCKS_LIB):
	+${MAKE_LIB};
endif

clean:
	rm -rf tmp libh2.a
	+for dir in ${LIB_DIRS}; do\
		make -C $$dir clean;\
	done;

libh2.a: ${ALL_LIBS}
	cd tmp;\
	for lib in $(notdir ${ALL_LIBS}); do\
		hexagon-ar x $$lib;\
	done;\
	hexagon-ar cru ../libh2.a *.o

#  tar is used instead of cp so we can recursively copy and also exclude CVS
#  Also, /pkg/gnu/bin/tar doesn't seem to work right.

install: libh2.a $(SYS_WRAPPER_LIB) $(LOCKS_LIB)
	mkdir -p $(INSTALLPATH)/lib;
	mkdir -p $(INSTALLPATH)/include;
	cp $^ $(INSTALLPATH)/lib;
	for dir in $(dir ${LIB_DIRS}); do\
		if [ -d "$$dir/include" ]; then \
			pushd $$dir/include;\
			/bin/tar cf - * | /bin/tar xvf - -C $(INSTALLPATH)/include --exclude '*CVS*';\
			popd;\
		fi;\
	done;
