#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  // 没查到为啥要符号扩展，但事实如此
  // 最新的manual里写了。。
  rtl_sext(&t0, &id_dest->val, id_dest->width);
  rtl_push(&t0);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);
  operand_write(id_dest, &t0);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  TODO();

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_msb(&t0, &cpu.eax, 2);
    if (t0 == 1)
      cpu.edx |= 0xffff;
    else
      cpu.edx &= 0xffff0000; 
  }
  else {
    if ((int32_t)cpu.eax < 0)
      cpu.edx = 0xffffffff;
    else
      cpu.edx = 0;
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_msb(&t0, &cpu.eax, 1);
    if (t0 == 1)
      cpu.eax |= 0xff00;
    else
      cpu.eax &= 0xffff00ff; 
  }
  else {
    rtl_msb(&t0, &cpu.eax, 2);
    if (t0 == 1)
      cpu.eax |= 0xffff0000;
    else
      cpu.eax &= 0x0000ffff;
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
