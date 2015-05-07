
** EJP 20150507 ** This is just copied over from qurt tree for the moment... needs rewriting


Note:
This build can only be run in linux environment
In a SUSE host(such as login-iceng-02), make sure below paths are added to the environment variable LD_LIBRARY_PATH
/pkg/qct/software/gnu/gmp/4.2.1/lib:/pkg/qct/software/gnu/mpfr/2.3.0/lib:/pkg/qct/software/gnu/gmp/4.1.4/lib


Instructions to build osam T32:
cd to trace32EDK_linux, 
do ./bin/make qurt_model.t32
the binary will appear in same dir
