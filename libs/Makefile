# Smaller kludge here

ifeq ($(ARCHV),)
export ARCHV := 3
endif

SHELL=/bin/bash

ifeq (,$(IMPL))
IMPL=ref
endif

ifeq (ref,$(IMPL))
H2_LIB=h2/build/ref/libh2.a
else
IMPL=opt
H2_LIB=h2/build/opt_v$(ARCHV)/libh2.a
endif

ALL_LIBS= $(H2_LIB) qube/libqube.a syscall/angel/libangel.a blast/libblast.a posix/libposix.a timer/libtimer.a

MAKE_LIB=${MAKE} -C $(dir $@); cp $@ tmp/


tmp:
	mkdir $@

$(H2_LIB): tmp
	+${MAKE} -C h2 $(IMPL);

blast/libblast.a: tmp
	+${MAKE_LIB};

qube/libqube.a: tmp
	+${MAKE_LIB};

posix/libposix.a: tmp
	+${MAKE_LIB};

syscall/angel/libangel.a: tmp
	+${MAKE_LIB};

timer/libtimer.a: tmp
	+${MAKE_LIB};


clean:
	rm -rf tmp libh2.a
	+for dir in $(dir ${ALL_LIBS}); do\
		make -C $$dir clean;\
	done;

libh2.a: ${ALL_LIBS}
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
