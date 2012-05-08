
all: hello booter

hello: hello.c
	qdsp6-gcc -Os -mv$(ARCHV) -moslib=h2 -I../install/include -L../install/lib -Wl,--section-start=.start=0x00010000 -o hello hello.c 

booter: booter.c elf.c
	qdsp6-gcc -Os -mv$(ARCHV) -o booter booter.c elf.c -moslib=h2 -lh2kernel -I../install/include -L../install/lib -Wl,--section-start=.start=0xff000000 -Wl,--defsym=HEAP_SIZE=0x100000

test: hello booter
	qdsp6-sim -- booter hello

install: booter
	mkdir -p $(INSTALLPATH)/bin
	cp booter $(INSTALLPATH)/bin

clean:
	rm -f *.elf booter hello stats.* 
