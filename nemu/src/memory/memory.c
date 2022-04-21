#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int no = is_mmio(addr);
  if (no == -1)
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  return mmio_read(addr, len, no);
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int no = is_mmio(addr);
  if (no == -1)
    memcpy(guest_to_host(addr), &data, len);
  else
    mmio_write(addr, len, data, no);
}

#define PDX(va)     (((uint32_t)(va) >> 22) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> 12) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)

paddr_t page_translate(vaddr_t addr, bool dirty) {
  if (cpu.cr0.val >> 31 != 1)
    return addr;
  PDE* pgdir = (PDE*)(cpu.cr3.val & 0xfffff000);
  PDE pde;
  pde.val = paddr_read((paddr_t)&(pgdir[PDX(addr)]), 4);
  // Log("%d", pde.val);
  assert(pde.present);
  pde.accessed = true;
  PTE* ptep = (PTE*)(uint32_t)(pde.val & 0xfffff000);
  PTE pte;
  pte.val = paddr_read((paddr_t)&(ptep[PTX(addr)]), 4);
  assert(pte.present);
  pte.accessed = true;
  pte.dirty = dirty;
  return (pte.val & ~0xfff) | OFF(addr);
}


uint32_t vaddr_read(vaddr_t addr, int len) {
  uint32_t ret = 0;
  if ((addr & 0xfffff000) != ((addr + len - 1) & 0xfffff000)){
    for (int i = 0; i < len; i++) {
      paddr_t paddr = page_translate(addr + i, false);
      ret |= paddr_read(paddr, 1) << (8 * i); 
    }
    if(addr > 0x1900000 && ret)
    Log("1:%x %x", addr, ret);
    return ret;
  }
  paddr_t paddr = page_translate(addr, false);
  ret = paddr_read(paddr, len);
    if(addr > 0x1900000 && ret)

  Log("2:%x %x", addr, ret);
  return ret;
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if ((addr & 0xfffff000) != ((addr + len - 1) & 0xfffff000))
    assert(0);
  paddr_t paddr = page_translate(addr, true);
  paddr_write(paddr, len, data);
}

