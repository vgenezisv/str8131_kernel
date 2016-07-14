cmd_arch/arm/lib/ucmpdi2.o := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -Wp,-MD,arch/arm/lib/.ucmpdi2.o.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian -D__ASSEMBLY__ -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=4 -march=armv4 -mtune=arm9tdmi -msoft-float    -c -o arch/arm/lib/ucmpdi2.o arch/arm/lib/ucmpdi2.S

deps_arch/arm/lib/ucmpdi2.o := \
  arch/arm/lib/ucmpdi2.S \
    $(wildcard include/config/aeabi.h) \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/linux/linkage.h \
  include/asm/linkage.h \

arch/arm/lib/ucmpdi2.o: $(deps_arch/arm/lib/ucmpdi2.o)

$(deps_arch/arm/lib/ucmpdi2.o):
