
all: hello booter

hello: hello.c
	qdsp6-gcc -Os -mv4 -moslib=h2 -I../install/include -L../install/lib -o hello hello.c 

booter: booter.c elf.c
	qdsp6-gcc -Os -mv4 -o booter booter.c elf.c -moslib=h2 -lh2kernel -I../install/include -L../install/lib -Wl,--section-start=.start=0xff000000 -Wl,--defsym=HEAP_SIZE=0x100000

test: hello booter
	qdsp6-sim -- booter hello

clean:
	rm -f *.elf booter hello stats.* 
