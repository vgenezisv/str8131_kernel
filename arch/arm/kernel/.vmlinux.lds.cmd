cmd_arch/arm/kernel/vmlinux.lds := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -E -Wp,-MD,arch/arm/kernel/.vmlinux.lds.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian  -DTEXT_OFFSET=0x00008000 -P -C -Uarm -D__ASSEMBLY__ -o arch/arm/kernel/vmlinux.lds arch/arm/kernel/vmlinux.lds.S

deps_arch/arm/kernel/vmlinux.lds := \
  arch/arm/kernel/vmlinux.lds.S \
    $(wildcard include/config/xip/kernel.h) \
    $(wildcard include/config/xip/phys/addr.h) \
    $(wildcard include/config/cpu/ispad/enable.h) \
  include/asm-generic/vmlinux.lds.h \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/asm/thread_info.h \
  include/linux/compiler.h \
  include/asm/fpstate.h \
    $(wildcard include/config/iwmmxt.h) \
  include/asm/memory.h \
    $(wildcard include/config/discontigmem.h) \
  include/asm/arch/memory.h \
    $(wildcard include/config/system/dram/base.h) \
    $(wildcard include/config/str8131/dram/64m.h) \
    $(wildcard include/config/system/dram/size.h) \
    $(wildcard include/config/str8131/dram/32m.h) \
    $(wildcard include/config/str8131/dram/16m.h) \
  include/asm/sizes.h \
  include/asm/cache.h \
    $(wildcard include/config/arch/str8131.h) \

arch/arm/kernel/vmlinux.lds: $(deps_arch/arm/kernel/vmlinux.lds)

$(deps_arch/arm/kernel/vmlinux.lds):
