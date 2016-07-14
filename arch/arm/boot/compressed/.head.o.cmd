cmd_arch/arm/boot/compressed/head.o := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -Wp,-MD,arch/arm/boot/compressed/.head.o.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian -D__ASSEMBLY__ -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=4 -march=armv4 -mtune=arm9tdmi -msoft-float    -c -o arch/arm/boot/compressed/head.o arch/arm/boot/compressed/head.S

deps_arch/arm/boot/compressed/head.o := \
  arch/arm/boot/compressed/head.S \
    $(wildcard include/config/debug/icedcc.h) \
    $(wildcard include/config/arch/sa1100.h) \
    $(wildcard include/config/debug/ll/ser3.h) \
    $(wildcard include/config/arch/iop331.h) \
    $(wildcard include/config/arch/s3c2410.h) \
    $(wildcard include/config/s3c2410/lowlevel/uart/port.h) \
    $(wildcard include/config/arch/str9100.h) \
    $(wildcard include/config/arch/str8131.h) \
    $(wildcard include/config/arch/cetusplus.h) \
    $(wildcard include/config/zboot/rom.h) \
    $(wildcard include/config/arch/rpc.h) \
    $(wildcard include/config/cpu/fa520.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/cpu/fa626.h) \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/linux/linkage.h \
  include/asm/linkage.h \
  include/asm/cache.h \

arch/arm/boot/compressed/head.o: $(deps_arch/arm/boot/compressed/head.o)

$(deps_arch/arm/boot/compressed/head.o):
