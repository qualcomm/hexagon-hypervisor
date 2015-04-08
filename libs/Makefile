# Smaller kludge here

ifeq ($(ARCHV),)
export ARCHV := 3
endif

SHELL=/bin/bash

ifeq (,$(IMPL))
IMPL=ref
endif

SOS_DIR=sos/
SOS_LIB=sos/build/ref/libsos.a

H2_DIR=h2/
ifeq (ref,$(IMPL))
H2_LIB=$(H2_DIR)build/ref/libh2.a
else
IMPL=opt
H2_LIB=$(H2_DIR)build/opt_v$(ARCHV)/libh2.a
endif

#MOST_LIBS= qube/libqube.a syscall/angel/libangel.a qurt/libqurt.a 
MOST_LIBS= syscall/angel/libangel.a qurt/libqurt.a #posix/libposix.a

ALL_LIBS= $(H2_LIB) $(SOS_LIB) $(MOST_LIBS)
LIB_DIRS= $(dir $(MOST_LIBS) $(H2_DIR) $(SOS_DIR))

MAKE_LIB=${MAKE} -C $(dir $@); cp $@ tmp/


tmp:
	mkdir $@

$(H2_LIB): tmp
	+${MAKE} -C $(H2_DIR) $(IMPL); cp $@ tmp/

blast/libblast.a: tmp $(H2_LIB)
	+${MAKE_LIB};

syscall/angel/libangel.a: tmp $(H2_LIB)
	+${MAKE_LIB};

$(SOS_LIB): tmp $(H2_LIB)
	+${MAKE} -C $(SOS_DIR) ref; cp $@ tmp/

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

install: libh2.a
	mkdir -p $(INSTALLPATH)/lib;
	mkdir -p $(INSTALLPATH)/include;
	cp $< $(INSTALLPATH)/lib;
	for dir in $(dir ${LIB_DIRS}); do\
		pushd $$dir/include;\
		/bin/tar cf - * | /bin/tar xvf - -C $(INSTALLPATH)/include --exclude '*CVS*';\
		popd;\
	done;
