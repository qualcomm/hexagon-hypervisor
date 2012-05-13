# set targetargs --rtos /prj/dsp/qdsp6/users/bryanb/h2/linux/qdsp6_linux.cfg --cosim_file /prj/dsp/qdsp6/users/bryanb/h2/linux/cosim.cfg

cd ~bryanb/pwa/h2/linux

set targetargs --cosim_file cosim.cfg

dir ~bryanb/pwa/linux-hexagon-kernel/busybox-1.17.4:~bryanb/pwa/linux-hexagon-kernel/uClibc/uClibc-0.9.30

add vmlinux 0
add busybox 0
