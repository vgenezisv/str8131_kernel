cmd_arch/arm/lib/copy_to_user.o := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -Wp,-MD,arch/arm/lib/.copy_to_user.o.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian -D__ASSEMBLY__ -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=4 -march=armv4 -mtune=arm9tdmi -msoft-float    -c -o arch/arm/lib/copy_to_user.o arch/arm/lib/copy_to_user.S

deps_arch/arm/lib/copy_to_user.o := \
  arch/arm/lib/copy_to_user.S \
  include/linux/linkage.h \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/asm/linkage.h \
  include/asm/assembler.h \
  include/asm/ptrace.h \
    $(wildcard include/config/arm/thumb.h) \
    $(wildcard include/config/smp.h) \
  arch/arm/lib/copy_template.S \

arch/arm/lib/copy_to_user.o: $(deps_arch/arm/lib/copy_to_user.o)

$(deps_arch/arm/lib/copy_to_user.o):
