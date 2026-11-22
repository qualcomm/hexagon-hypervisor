# Smaller kludge here

include ../scripts/Makefile.inc.install

ifeq ($(ARCHV),)
export ARCHV := 68
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

# Picolibc support: if PICOLIBC=1, every library is built against Picolibc (flags
# from scripts/Makefile.inc.libc, OSLIB_CFLAGS, through each component's
# TARGET_FLAG), and this build also builds the syscall wrapper and locks.  The
# Picolibc tree is separate from the Dinkumware one, so the aggregate libh2.a only
# ever links Picolibc guests.
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

TMP_DIR := $(INSTALLPATH)/../build/libs/tmp

MAKE_LIB=${MAKE} -C $(dir $@)

# The sub-makes know their sources, so always run them (FORCE); an archive is
# only rewritten when something in it changed.
.PHONY: FORCE
FORCE:


$(TMP_DIR):
	mkdir -p $@

$(H2_LIB): FORCE
	+${MAKE} -C $(H2_DIR) $(BUILD_TYPE)

$(POSIX_LIB): $(H2_LIB) FORCE
	+${MAKE} -C posix $(BUILD_TYPE)

$(H2_COMPAT_LIB): $(H2_LIB) $(POSIX_LIB) FORCE
	+${MAKE} -C $(H2_COMPAT_DIR) $(BUILD_TYPE)

$(QURT_LIB): $(H2_LIB) $(H2_COMPAT_LIB) $(POSIX_LIB) FORCE
	+${MAKE} -C qurt

$(ANGEL_LIB): $(H2_LIB) $(H2_COMPAT_LIB) $(POSIX_LIB) FORCE
	+${MAKE} -C syscall/angel

ifeq ($(PICOLIBC),1)
$(SYS_WRAPPER_LIB): FORCE
	+${MAKE} -C syscall/wrapper

$(LOCKS_LIB): FORCE
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
	rm -f $(TMP_DIR)/*.o $(TMP_DIR)/libh2.a
	cd $(TMP_DIR); \
	for lib in $(ALL_LIBS); do \
		hexagon-ar x $$lib; \
	done; \
	hexagon-ar cru libh2.a *.o


include $(H2DIR)/scripts/Makefile.inc.headers

# Before 'install', which expands $(INSTALL_TARGETS) when parsed.
$(call install_files,$(INSTALLPATH)/lib,$(TMP_DIR)/libh2.a $(SYS_WRAPPER_LIB) $(LOCKS_LIB))

install: $(INSTALL_TARGETS) $(INSTALLED_HEADERS)
