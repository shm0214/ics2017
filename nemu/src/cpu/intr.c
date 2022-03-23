#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  rtl_push(&cpu.eflags);
  rtl_push(&cpu.CS);
  rtl_push(&ret_addr);
  rtl_li(&t0, vaddr_read(cpu.idtr.base + NO * 8, 2));
  rtl_li(&t1, vaddr_read(cpu.idtr.base + NO * 8 + 6, 2));
  decoding.jmp_eip = (t1<<16)| (0xffff&t0);
  decoding.is_jmp = 1;
  
}

void dev_raise_intr() {
}
