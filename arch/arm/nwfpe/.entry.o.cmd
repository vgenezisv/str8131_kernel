cmd_arch/arm/nwfpe/entry.o := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -Wp,-MD,arch/arm/nwfpe/.entry.o.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian -D__ASSEMBLY__ -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=4 -march=armv4 -mtune=arm9tdmi -msoft-float    -c -o arch/arm/nwfpe/entry.o arch/arm/nwfpe/entry.S

deps_arch/arm/nwfpe/entry.o := \
  arch/arm/nwfpe/entry.S \

arch/arm/nwfpe/entry.o: $(deps_arch/arm/nwfpe/entry.o)

$(deps_arch/arm/nwfpe/entry.o):
