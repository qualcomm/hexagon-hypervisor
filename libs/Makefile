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

# Picolibc support: if PICOLIBC=1, build syscall wrapper and locks.
# The Picolibc compiler flags themselves live in each of those component
# Makefiles (posix, syscall/wrapper, locks) via their own TARGET_FLAG.  They are
# intentionally NOT applied to h2, h2_compat, qurt, or syscall/angel: those are
# combined into the aggregate libh2.a that is linked into ordinary
# (default-libc) guest programs, where a Picolibc-specific ABI (e.g. TLS errno)
# would fail to resolve.
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

# $(call lib_sources,<dir>) -> every source file under a sub-library.
#
# Only the sub-make knows a library's real file list; it is spread over the
# make.inc fragments under each module directory.  Every file under the library's
# directory is a superset of that list, which errs the safe way: the parent may
# consult a child that had nothing to do (it no-ops, its archive's mtime does not
# move, nothing downstream is invalidated), but never misses a real edit.
lib_sources = $(shell find $(1) -type f ! -name '*.o' ! -name '*.a' ! -name '*.d')

$(H2_LIB): $(call lib_sources,$(H2_DIR))
	+${MAKE} -C $(H2_DIR) $(BUILD_TYPE)

# Ordinary prerequisites, not order-only: posix, h2_compat, qurt and angel compile
# against each other's generated headers, and a child is only consulted when its
# own directory changed -- order-only would hide h2 header edits from them.
# FIXME: a header-only edit in h2/ leaves $(H2_LIB) unchanged (only the generated
# header copy moves), so posix/h2_compat/qurt/angel are never re-consulted and
# keep objects built against the old header.  Depending on the h2 sources as
# well would fix it.
$(POSIX_LIB): $(H2_LIB) $(call lib_sources,$(POSIX_DIR))
	+${MAKE} -C posix $(BUILD_TYPE)

$(H2_COMPAT_LIB): $(H2_LIB) $(POSIX_LIB) $(call lib_sources,$(H2_COMPAT_DIR))
	+${MAKE} -C $(H2_COMPAT_DIR) $(BUILD_TYPE)

$(QURT_LIB): $(H2_LIB) $(H2_COMPAT_LIB) $(POSIX_LIB) $(call lib_sources,qurt)
	+${MAKE} -C qurt

$(ANGEL_LIB): $(H2_LIB) $(H2_COMPAT_LIB) $(POSIX_LIB) $(call lib_sources,syscall/angel)
	+${MAKE} -C syscall/angel

ifeq ($(PICOLIBC),1)
$(SYS_WRAPPER_LIB): $(call lib_sources,syscall/wrapper)
	+${MAKE} -C syscall/wrapper

$(LOCKS_LIB): $(call lib_sources,locks)
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

$(call install_files,$(INSTALLPATH)/lib,$(TMP_DIR)/libh2.a $(SYS_WRAPPER_LIB) $(LOCKS_LIB))

install: $(INSTALL_TARGETS)
	mkdir -p $(INSTALLPATH)/include;
	for dir in $(_H2_HDR_DIR) $(_POSIX_HDR_DIR) $(_H2COMPAT_HDR_DIR) syscall/angel/include qurt/include; do\
		if [ -d "$$dir" ]; then \
			pushd $$dir;\
			/bin/tar cf - * | /bin/tar xvf - -C $(INSTALLPATH)/include --exclude '*CVS*';\
			popd;\
		fi;\
	done;
