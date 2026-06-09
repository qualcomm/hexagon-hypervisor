# Smaller kludge here

ifeq ($(ARCHV),)
export ARCHV := 3
endif

SHELL=/bin/bash

# Derive lib archive paths and tmp staging dir from INSTALLPATH.
# Use $(INSTALLPATH)/../build/... so this works for any INSTALLPATH value,
# not just ones ending in /install.
H2_LIB    := $(INSTALLPATH)/../build/libs/h2/libh2.a
H2_COMPAT_LIB := $(INSTALLPATH)/../build/libs/h2_compat/libh2_compat.a
POSIX_LIB := $(INSTALLPATH)/../build/libs/posix/libposix.a

QURT_LIB  := $(INSTALLPATH)/../build/libs/qurt/libqurt.a
ANGEL_LIB := $(INSTALLPATH)/../build/libs/syscall/angel/libangel.a

H2_DIR=h2/
H2_COMPAT_DIR=h2_compat/
POSIX_DIR=posix/

#MOST_LIBS= qube/libqube.a syscall/angel/libangel.a qurt/libqurt.a
MOST_LIBS= $(ANGEL_LIB) $(QURT_LIB)

# Picolibc support: if PICOLIBC=1, build syscall wrapper and locks
ifeq ($(PICOLIBC),1)
export PICOLIBC
SYS_WRAPPER_LIB = $(INSTALLPATH)/../build/libs/syscall/wrapper/libsyscall_wrapper.a
LOCKS_LIB       = $(INSTALLPATH)/../build/libs/locks/liblocks.a
else
SYS_WRAPPER_LIB =
LOCKS_LIB =
endif

ALL_LIBS= $(H2_LIB) $(POSIX_LIB) $(H2_COMPAT_LIB) $(MOST_LIBS)
LIB_DIRS= $(dir $(MOST_LIBS) $(H2_DIR) $(POSIX_DIR) $(H2_COMPAT_DIR) $(SYS_WRAPPER_LIB) $(LOCKS_LIB))

# Source directories used for installing headers (not build dirs).
HEADER_DIRS = $(H2_DIR) $(POSIX_DIR) $(H2_COMPAT_DIR) syscall/angel/ qurt/

TMP_DIR := $(INSTALLPATH)/../build/libs/tmp

# Per-variant header dirs for the three generated-header sub-libs.
_H2_HDR_DIR       := $(INSTALLPATH)/../build/libs/h2/include
_POSIX_HDR_DIR    := $(INSTALLPATH)/../build/libs/posix/include
_H2COMPAT_HDR_DIR := $(INSTALLPATH)/../build/libs/h2_compat/include

MAKE_LIB=${MAKE} -C $(dir $@)


$(TMP_DIR):
	mkdir -p $@

$(H2_LIB):
	+${MAKE} -C $(H2_DIR) $(BUILD_TYPE)

$(POSIX_LIB): $(H2_LIB)
	+${MAKE} -C posix $(BUILD_TYPE)

$(H2_COMPAT_LIB): $(H2_LIB) $(POSIX_LIB)
	+${MAKE} -C $(H2_COMPAT_DIR) $(BUILD_TYPE)

$(QURT_LIB): $(H2_LIB) $(H2_COMPAT_LIB) $(POSIX_LIB)
	+${MAKE} -C qurt

$(ANGEL_LIB): $(H2_LIB) $(H2_COMPAT_LIB) $(POSIX_LIB)
	+${MAKE} -C syscall/angel

ifeq ($(PICOLIBC),1)
$(SYS_WRAPPER_LIB):
	+${MAKE} -C syscall/wrapper

$(LOCKS_LIB):
	+${MAKE} -C locks
endif

clean:
	rm -rf $(TMP_DIR)
	+for dir in $(H2_DIR) $(POSIX_DIR) $(H2_COMPAT_DIR) qurt/ syscall/angel/; do\
		make -C $$dir clean;\
	done;

# Combine all component archives into a single libh2.a.  All paths are
# absolute so cd $(TMP_DIR) doesn't invalidate them.
$(TMP_DIR)/libh2.a: ${ALL_LIBS} | $(TMP_DIR)
	rm -f $(TMP_DIR)/*.o
	cd $(TMP_DIR); \
	for lib in $(ALL_LIBS); do \
		hexagon-ar x $$lib; \
	done; \
	hexagon-ar cru libh2.a *.o

#  tar is used instead of cp so we can recursively copy and also exclude CVS
#  Also, /pkg/gnu/bin/tar doesn't seem to work right.

install: $(TMP_DIR)/libh2.a $(SYS_WRAPPER_LIB) $(LOCKS_LIB)
	mkdir -p $(INSTALLPATH)/lib;
	mkdir -p $(INSTALLPATH)/include;
	cp $^ $(INSTALLPATH)/lib;
	for dir in $(_H2_HDR_DIR) $(_POSIX_HDR_DIR) $(_H2COMPAT_HDR_DIR) syscall/angel/include qurt/include; do\
		if [ -d "$$dir" ]; then \
			pushd $$dir;\
			/bin/tar cf - * | /bin/tar xvf - -C $(INSTALLPATH)/include --exclude '*CVS*';\
			popd;\
		fi;\
	done;
