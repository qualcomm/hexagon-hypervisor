
ifeq ($(ARCHV),)
ARCHV=3
endif

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(PWD)/install
endif

clean:
	make -C kernel ARCHV=$(ARCHV) clean
	make -C libs ARCHV=$(ARCHV) clean

opt: clean
	make -C kernel ARCHV=$(ARCHV) opt_install
	make -C libs ARCHV=$(ARCHV) install

ref: clean
	make -C kernel ARCHV=$(ARCHV) ref_install
	make -C libs ARCHV=$(ARCHV) install

compat:
	cd install/lib ; ln -s libh2kernel.a libblastkernel.a ; ln -s libh2.a libblast.a


