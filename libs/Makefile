
# Big kludge here, need to redo build system to have one h2 library built

all: libh2

libh2:
	make -C h2
	make -C syscall/angel
	mkdir -p tmp
	cp h2/libh2.a tmp/
	cp syscall/angel/libangel.a tmp/
	cd tmp; qdsp6-ar x libh2.a; qdsp6-ar x libangel.a; qdsp6-ar cru ../libh2.a *.o

clean:
	rm libh2.a tmp
