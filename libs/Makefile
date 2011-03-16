# Smaller kludge here

ifeq ($(ARCHV),)
export ARCHV := 3
endif

SHELL=/bin/bash

all: libh2.a

ALL_LIBS=h2/libh2.a qube/libqube.a syscall/angel/libangel.a blast/libblast.a posix/libposix.a 

MAKE_LIB=${MAKE} -C $(dir $@); cp $@ tmp/


tmp:
	mkdir $@

blast/libblast.a: tmp
	+${MAKE_LIB};

qube/libqube.a: tmp
	+${MAKE_LIB};

posix/libposix.a: tmp
	+${MAKE_LIB};

syscall/angel/libangel.a: tmp
	+${MAKE_LIB};

h2/libh2.a: tmp
	+${MAKE_LIB};

clean:
	rm -rf tmp libh2.a
	+for dir in $(dir ${ALL_LIBS}); do\
		make -C $$dir clean;\
	done;

libh2.a:   ${ALL_LIBS}
	cd tmp;\
	for lib in $(notdir ${ALL_LIBS}); do\
		qdsp6-ar x $$lib;\
	done;\
	qdsp6-ar cru ../libh2.a *.o

#  tar is used instead of cp so we can recursively copy and also exclude CVS
#  Also, /pkg/gnu/bin/tar doesn't seem to work right.

install: libh2.a
	mkdir -p $(INSTALLPATH)/lib;
	mkdir -p $(INSTALLPATH)/include;
	cp $< $(INSTALLPATH)/lib;
	for dir in $(dir ${ALL_LIBS}); do\
		pushd $$dir/include;\
		/bin/tar cf - * | /bin/tar xvf - -C $(INSTALLPATH)/include --exclude '*CVS*';\
		popd;\
	done;


