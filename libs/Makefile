
# Big kludge here, need to redo build system to have one blast library built

all: libblast

libblast:
	make -C blast
	make -C syscall/angel
	mkdir -p tmp
	cp blast/libblast.a tmp/
	cp syscall/angel/libangel.a tmp/
	cd tmp; qdsp6-ar x libblast.a; qdsp6-ar x libangel.a; qdsp6-ar cru ../libblast.a *.o

