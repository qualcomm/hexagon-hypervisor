
void  H2K_vm_do_ipi(H2K_thread_context *me);

clear bit in globals ipimask
if ipi mask non-zero, setimask for ipi interrupt with ~ipimask and reraise

void H2K_vm_send_ipi(H2K_thread_context *dest);

set bit in globals ipimask
setimask for ipi interrupt with ~ipimask and raise


