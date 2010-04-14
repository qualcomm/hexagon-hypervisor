# Smaller kludge here

ifeq ($(ARCHV),)
export ARCHV := 3
endif

all: libh2.a

ALL_LIBS=h2/libh2.a qube/libqube.a posix/libposix.a syscall/angel/libangel.a blast/libblast.a

MAKE_LIB=${MAKE} -C $(dir $@); cp $@ tmp/
	

blast/libblast.a:
	${MAKE_LIB};

qube/libqube.a:
	${MAKE_LIB};

posix/libposix.a:
	${MAKE_LIB};

syscall/angel/libangel.a:
	${MAKE_LIB};

h2/libh2.a:
	${MAKE_LIB};

clean:
	for dir in $(dir ${ALL_LIBS}); do\
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
	OWD=`pwd`;\
	for dir in $(dir ${ALL_LIBS}); do\
		pushd $$dir/include;\
		/bin/tar cf - * | /bin/tar xvf - -C $$OWD/../install/include --exclude '*CVS*';\
		popd;\
	done;



