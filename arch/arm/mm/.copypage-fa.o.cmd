cmd_arch/arm/mm/copypage-fa.o := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -Wp,-MD,arch/arm/mm/.copypage-fa.o.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian -D__ASSEMBLY__ -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=4 -march=armv4 -mtune=arm9tdmi -msoft-float    -c -o arch/arm/mm/copypage-fa.o arch/arm/mm/copypage-fa.S

deps_arch/arm/mm/copypage-fa.o := \
  arch/arm/mm/copypage-fa.S \
    $(wildcard include/config/cpu/dcache/writethrough.h) \
  include/linux/linkage.h \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/asm/linkage.h \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/memory/hotplug.h) \
  include/linux/compiler.h \
  include/asm/asm-offsets.h \

arch/arm/mm/copypage-fa.o: $(deps_arch/arm/mm/copypage-fa.o)

$(deps_arch/arm/mm/copypage-fa.o):
