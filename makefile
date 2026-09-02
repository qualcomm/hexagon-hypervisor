# Fool the q6auto jenkins stupidity: That stuff doesn't actually run
# your makefile directly; they include it in *their* makefile and then
# bsub that with the environment stripped (apparently to keep project
# makefiles from messing with their stuff or LSF envars).  The result
# of that is that you can't 'export VAR' anything from the top-level
# make to your submakes. :/

.PHONY: q6testinstallenvs
q6testinstallenvs:
#	export Q6TESTINSTALL_MAKEJOBS=8
	export Q6TESTINSTALL_TESTTARGET=bigtest


%:
	$(MAKE) -f makefile.real $@
